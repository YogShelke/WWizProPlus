
// WrdWizDownloaderDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "WinHttpManager.h"
#include "TextProgressCtrl.h"
#include "afxcmn.h"
#include "SimpleBrowser.h"


#define TIMER_SETPERCENTAGE 400
#define TIMER_FORADVERTISEMENT 600
#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30
#define WM_TRAYMESSAGE WM_USER

class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWrdWizDownloaderDlg dialog
class CWrdWizDownloaderDlg : public CJpegDialog,
	public CSciterBase,
	public sciter::host<CWrdWizDownloaderDlg> // Sciter host window primitives
{
// Construction
public:
	CWrdWizDownloaderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WRDWIZDOWNLOADER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
public:
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();							 
	bool CheckInternetConnection();
	bool WaitForInternetConnection();
	bool StartDownloadingWardWizSetup(LPCTSTR szUrlPath);

	afx_msg void OnBnClickedButtonRun();
	CxSkinButton m_btndwnldpathBrowse;
	afx_msg void OnBnClickedButtonDownloadBrowse();
	CEdit m_edtBrowsedSetupPath;
	CxSkinButton m_btnPause;
	CxSkinButton m_btnResume;
	CxSkinButton m_btnClose;
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonResume();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CTextProgressCtrl m_prgDownloadSetupStatus;
	DWORD GetPercentage(int iDownloaded, int iTotalSize);
	void IsWow64();
	bool m_bIsWow64;
	
	afx_msg void OnClose();
	CButton m_chkLaunchDownloadedExe;
	CButton m_chkOpenFolderOption;
	afx_msg void OnBnClickedCheckLaunch();
	afx_msg void OnBnClickedCheckOpenfolder();
	CString m_csFileTargetPath;
	bool m_bchkOpenFolder;
	bool m_bchkLaunchExe;
	bool m_bchkDownloadoption;
	CString m_csActualProdName;
	void HideAllElements();
	
	//CButton m_radio32bitSetup;
	//CButton m_radio64bitSetup;

	TCHAR m_szTempFolderPath[512];
	TCHAR m_szIniFilePath[512];
	CString m_csTempTargetFilePath;

	//afx_msg void OnBnClickedRadio32bitSetup();
	//afx_msg void OnBnClickedRadio64bitSetup();
	
	
	//afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void SetStartupEntry(TCHAR* szDownloaderPath);
	bool GetStartupEntry();
	CColorStatic m_stSelectDwnldText;
	CColorStatic m_grpFirstBox;
	CColorStatic m_stDwnldInformation;
	CColorStatic m_grpSecondBox;
	CColorStatic m_stProductName;
	CColorStatic m_stActualProdName;
	CColorStatic m_stDwnldPath;
	CColorStatic m_stActualDownloadPath;
	CColorStatic m_stTransferRate;
	CColorStatic m_stActualTransferRate;
	CColorStatic m_stRemainingTime;
	CColorStatic m_stActualRemainingTime;
	CColorStatic m_stStatus;
	CColorStatic m_stActualStatus;
	CColorStatic m_grpThirdBox;
	CButton m_chkDownloadOption;
	CStatic m_stFooterText;
	CTime	m_tsStartTime;
	bool	m_bIsDownloadingInProgress;
	TCHAR	m_szModulePath[512];
	CWinHttpManager m_objWinHttpManager;
	DWORD m_dwTotalFileSize;
	HANDLE m_hFile;
	TCHAR m_szBrowseFolderName[512];
	HANDLE m_hStartWardWizSetupDwnldProc;
	int m_iCurrentDownloadedByte;
	DWORD m_dwPercentage;
	DWORD m_dwFilePath;
	CString m_csLangType;
	CxSkinButton m_btnRun;
	bool m_bAttimeRestart;
	//bool m_bMinimize;
	afx_msg void OnBnClickedCheckDwnldOption();
	VOID MinimizeWndToTray();
	BOOL GetDoAnimateMinimize(VOID);
	VOID GetTrayWndRect(LPRECT lpTrayRect);
	VOID ShowNotifyIcon(HWND hWnd, DWORD dwAdd, CString csMessage);
	VOID RestoreWndFromTray(HWND hWnd);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	CString SaveAsDoublicateFile(CString csfilePath);
	CString m_csFileName;
	CStatic m_stBrowser;
	void OnURL_Navigate(LPCTSTR URL);
	afx_msg void OnBeforeNavigate2(NMHDR *pNMHDR, LRESULT *pResult);
	//bool OnBeforeNavigate2(CString URL,
	//	CString frame,
	//	void    *post_data, int post_data_size,
	//	CString headers);
	//SimpleBrowser m_simplebrowser;
	
	//afx_msg void OnStnClickedStaticBroswer();
	CString m_csNavigateUrl;
	//DWORD					m_dwcount;
	CStatic m_stDownloaderLogo;
	HBITMAP m_bmpDownloaderLogoPic;
	HBITMAP m_bmpOfflineAdPic;
	CColorStatic m_stdownloadHeaderName;
	CxSkinButton m_btnSystemClose;
	CxSkinButton m_btnSystemMinimize;
	afx_msg void OnBnClickedButtonWindowClose();
	afx_msg void OnBnClickedButtonWindowMinimize();
	CColorStatic m_stLaunchText;
	CColorStatic m_stOpenFolderText;

	CStatic m_stOfflineAd;
	ULARGE_INTEGER					m_uliFreeBytesAvailable;    
	ULARGE_INTEGER					m_uliTotalNumberOfBytes;     
	ULARGE_INTEGER					m_uliTotalNumberOfFreeBytes; 
	void CancelUI();
	bool PauseDownload();
	bool ResumeDownload();
	bool m_bIsProcessPaused;
	bool m_bIsCloseCalled;
	bool IsDriveHaveRequiredSpace(CString csDrive, int iSpaceRatio, DWORD dwSetupFileSize);
	CString m_csDrivePath; 
	bool m_bRequiredSpaceNotavailable;
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	SCITER_VALUE					m_svFunUpdateDownloadStatusCB;
	SCITER_VALUE					m_svFunGetDownloadPathCB;
	SCITER_VALUE					m_svFunShowNotificationMsgCB;
	SCITER_VALUE					m_svFunCallOnCloseUICB;
	SCITER_VALUE					m_svFunGetProdLang;
	sciter::dom::element			m_root_el;



	BEGIN_FUNCTION_MAP
		FUNCTION_5("OnLoadDownloader", On_LoadDownloader)
		FUNCTION_0("OnClickPause", On_BtnClickPause)
		FUNCTION_0("OnClickResume", On_BtnClickResume)
		FUNCTION_0("OnClickMinimize", On_BtnClickMinimize)
		FUNCTION_0("OnClickClose", On_BtnClickClose)
		FUNCTION_1("GetString", Get_String);

	END_FUNCTION_MAP
	
	json::value On_LoadDownloader(SCITER_VALUE svFunGetProdLang, SCITER_VALUE svFunUpdateDownloadStatusCB, SCITER_VALUE svFunGetDownloadPathCB, SCITER_VALUE svFunShowNotificationMsgCB, SCITER_VALUE svFunCallOnCloseUICB);
	json::value On_BtnClickPause();
	json::value On_BtnClickResume();
	json::value On_BtnClickMinimize();
	json::value On_BtnClickClose();
	json::value Get_String(SCITER_VALUE svStringValue);
};

