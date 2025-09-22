/*******************************************************************************************
*  Program Name: ISpyLiveUpSecondDlg.h                                                                                                  
*  
*  Author Name:		1)Ramkrushna
*					2)Neha Gharge
*  Date Of Creation: 15-0ct 2013                                                                                              
*  Version No:		 1.0.0.2   
/*********************************************************************/
//HEADER FILE
/**********************************************************************/
#pragma once
#include <Winreg.h>
#include <time.h>
#include "pictureex.h"
#include "WinHttpManager.h"
#include "afxwin.h"
#include "ColorStatic.h"
#include "xSkinButton.h"
#include "GradientStatic.h"
#include "TextProgressCtrl.h"
#include "ISpyCriticalSection.h"
#include "EnumProcess.h"
#include "WrdWizCustomMsgBox.h"


/*********************************************************************
CLASS DECLARATION,FUCTION DECLARATION,MEMEBER VARIABLE DECLARATION

**********************************************************************/
enum eDownloadStatus
{
	PENDING			= 0,
	DOWNLOADING		= 1,
	PAUSED			= 2,
	RESUMED			= 3,
	FAILEDSTATUS	= 4,
	RECONNECTING	= 5,
	COMPLETED		= 6,
	CANCELLED		= 7
};

// CISpyLiveUpSecondDlg dialog

class CISpyLiveUpSecondDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyLiveUpSecondDlg)

public:
	CISpyLiveUpSecondDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyLiveUpSecondDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LIVEUPDATE_SECONDPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
public:
	virtual BOOL OnInitDialog();
	bool StartDownloading();
	bool GetApplicationDataFolder(TCHAR *szAppPath);
	bool CreateTempFolder();
	bool MakeDownloadUrlList();
	bool StartDownloadFile(LPCTSTR szUrlPath);
	DWORD GetPercentage(int iDownloaded, int iTotalSize);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void FillListControl(std::vector<CString> strVector);
	DWORD GetTotalFilesSize();
	//DWORD GetTotalExistingBytes();
	bool CopyDownloadedFiles2InstalledFolder();
	bool PauseDownload();
	bool ResumeDownload();
	bool StopDownloading();
	bool CheckInternetConnection();
	DWORD UpdateFromLocalFolder(std::vector<CString> &csVectInputFiles);
	DWORD CopyFromLocalFolder2InstalledFolder(std::vector<CString> &csVectInputFiles);
	void SetItemDownloadStatus(eDownloadStatus eStatus);
	bool ShutDownDownload();
	bool WaitForInternetConnection();
	void EnableWindows(bool bEnable);
	void AdjustControls();
	void UpdateTimeDate();
	//afx_msg void OnBnClickedSecButtonHelp();
	afx_msg void OnBnClickedButtonDwcancel();
	afx_msg void OnBnClickedBackButton();
	afx_msg void OnBnClickedButtonPause();
	void ShowUpdateCompleteMessage();
	void ResetControls();
	bool IsFileMismatch();
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait);
	bool SendLiveUpdateOperation2Service(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bLiveUpdateWait);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	bool InitImageList();
public:
	bool						m_bIsCheckingForUpdateEntryExistInList;
	CImageList					m_imageList;
	bool						m_bisDateTime;
	CPictureEx					m_Picture;
	HANDLE						m_hEvtFinished;
	CWinHttpManager				m_objWinHttpManager;
	TCHAR						m_szAppDataFolder[MAX_PATH];
	std::vector<CString>		m_vFileSizeMismatch;	
	std::vector<CString>		m_vUrlLists;
	CTextProgressCtrl			m_prgFileDownloadStatus;
	CListCtrl					m_lstFileDownloadStatus;
	DWORD						m_iFileSize;
	DWORD						m_iTotalFileSize;
	CColorStatic				m_stTotalFileSize;
	bool						m_bDownLoadInProgress;
	CTime						m_tsStartTime;
	//HANDLE						m_hThread;
	CColorStatic				m_stTotalDownload;
	CColorStatic				m_stDownloadSpeed;
	CColorStatic				m_stRemainingTime;
	int							m_iCurrentItemCount;
	CColorStatic				m_stTimeRemaining;
	CColorStatic				m_stSpeedHeader;
	CxSkinButton				m_btnCancel;
	CxSkinButton				m_btnPauseResume;
	HCURSOR						m_hButtonCursor;
	CxSkinButton				m_btnBack;
	CxSkinButton				m_btnSecHelp;
	CStatic						m_stSecHeaderPic;
	HBITMAP						m_bmpSecHeader;
	bool						m_isDownloading;
	int							m_iCurrentDownloadBytes;
	DWORD						m_dwCompletedBytes;
	DWORD						m_dwPercentage;
	DWORD						m_dwstatusMsg;
	CString						m_csListControlStatus;
	DWORD						m_dwTotalFileNo;
	DWORD						m_dwDownloadOrUpdateFileNo;
	int							m_iRowCount;
	TCHAR						m_szAllUserPath[512];
	bool						m_bManualStop;
	/* issue:165 Resolved by - lalit 9/2/2014 */
	bool                        m_bISalreadyDownloadAvailable;
	bool						m_bCloseUpdate;
	bool						m_bIsPopUpDisplayed;
	CString						m_stStaticInProgress;

	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CColorStatic m_stUpdatesSecondHeader;
	void RefreshStrings();
	CColorStatic m_stUpdatesSubHeader;
	
	afx_msg void OnBnClickedSecButtonStop();
	CColorStatic m_stDownloaded;
	CColorStatic m_stTotalSize;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void GoToHomePage();
	CxSkinButton m_btnResume;
	afx_msg void OnBnClickedButtonResume();
	afx_msg void OnPaint();
	bool SendRequestCommon(int iRequest);
	void InsertItem(CString csInsertItem , CString csActualStatus);
	void OnAddUpdateStatus(CString csStatus);
	void EnableControls(BOOL bEnable);
	bool UpDateDowloadStatus(LPISPY_PIPE_DATA lpSpyData);
	DECLARE_MESSAGE_MAP()
};
