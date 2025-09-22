#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <psapi.h>


// CISpyAntiRootkit dialog

class CISpyAntiRootkit : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyAntiRootkit)

public:
	CISpyAntiRootkit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyAntiRootkit();

// Dialog Data
	enum { IDD = IDD_DIALOG_ANTIROOTKIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int				 m_icount;
	bool             m_bCheckProcess;
	bool			 m_bCheckRegistry;
	bool			 m_bCheckFileFolder;
	DWORD			 m_dwGetCheckValues;
	CString			 m_csPreviousStatus;
	HANDLE           m_hGetPercentage;
	HANDLE			 m_hCleaningThread;
	bool			 m_bAntirootClose;
	bool			 m_bAntirootkitHome;
	bool             m_bAntirootkitScan;//Amit Dutta
	bool			 m_bRedFlag;
	HCURSOR			 m_hButtonCursor;
	//CxSkinButton	 m_btnBack;
	CStatic			 m_stHAntiRootkit;
	HBITMAP			 m_bmpAntiRootkit;
	CxSkinButton	 m_btnScan;
// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.

	CxSkinButton	 m_btnStop;
	CButton			 m_chkProcess;
	CButton			 m_chkRegistry;
	CButton			 m_chkFilesFolders;
	CColorStatic	 m_stProcess;
	CColorStatic	 m_stRegistry;
	CColorStatic	 m_stFilesFolders;
	CColorStatic	 m_stSelectionText;
	CFont			*m_FontText;
	CTextProgressCtrl m_prgAntiRootkit;
	CColorStatic     m_stPercentage;
	CColorEdit		 m_edtAntiRootkitStatus;
	bool			 m_bScanningFinished;
	CTabCtrl	     m_tabAntiRootkit;
	CListCtrl        m_lstAntiRookit;
	CxSkinButton     m_btnAntiRootkitDelete;
	CButton          m_chkAntiRootkitSelectAll;
	CColorStatic     m_stAntiRootkitSelectAll;
	CListCtrl        m_lstRegistry;
	CListCtrl        m_lstFilesFolders;
	CColorStatic     m_stScannedEntries;
	CColorStatic     m_stThreatsFound;
	CColorStatic	 m_stScannedRegistry;
	CColorStatic	 m_stThreatsRegistry;
	CColorStatic	 m_stScannedFilesFolders;
	CColorStatic	 m_stThreatsFilesFolders;
	DWORD			 m_dwScannedCnt;
	DWORD			 m_dwScannedFileFolder;
	DWORD			 m_dwScannedCntRegistry;
	DWORD			 m_dwThreatCntFilefolder;
	DWORD			 m_dwThreatCntRegistry;
	DWORD			 m_dwThreatCnt;
	CString			 m_csScannedEntries;
	CString		     m_csThreatsfound;
	bool		     m_bProcessTabSelected;
	bool			 m_bRegistryTabSelected;
	bool             m_bFilesFoldersTabSelected;
	CTime			 m_tsScanStartTime;
	CTimeSpan		 m_tsScanElapsedTime; 
	CTimeSpan		 m_tsScanPauseResumeElapsedTime;
	CTime			 m_tsScanPauseResumeTime;
	bool			 m_bRootkitDeleted;
	bool			 m_bScanAborted;
	bool			 m_bScanningStopped;
	DWORD			 m_dwRootKitOption;
	DWORD			 m_dwPercentage;
	DWORD			 m_dwType;
	DWORD			 m_dwTotalType;
	iSpyServerMemMap_Client m_objIPCRootkitClient;
	DWORD			 m_dwForInsertItem;
	bool			 m_bAntirootScanningInProgress;
	bool			 m_bAntirootkitCompleted;
	bool			 m_bOnWMClose;
	bool			 m_bIsPopUpDisplayed;

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void ShowHideControls(bool bEnable);
	//afx_msg void OnBnClickedBtnback();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedBtnScan();
	void SetScanStatus(CString csStatus);	
	void ShowAntiRootkitScannedList();	
	bool OnBnClickBtnSecondAntirootkitUI();
	void HideControls4ScannedList(bool bEnable);
	afx_msg void OnTcnSelchangeTabProcess(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnantirootkitDelete();
	void HideChildControls(bool bEnable);
	afx_msg void OnBnClickedCheckAntirootkitselectall();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void DeleteEntries();
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait = false);
	bool SendFile4RepairUsingService(int iMessage, CString csEntryOne, CString csEntryTwo, CString csEntryThree, DWORD dwISpyID , DWORD dwScanType, bool bWait = false);
	bool SendReportOperations2Service(int dwMessageinfo, CString csReportFileEntry ,DWORD dwType, bool bWait = false);
	bool StartAntirootScanUsingService();
	bool IsAllEntriesCleaned();
	bool ShutDownRootkitScanning();
	void GetDWORDFromRootKitScanOptions( DWORD &dwRegScanOpt);
	bool isAnyEntrySeletected();
	bool SendRequestCommon(int iRequest);
	bool RootKitScanningFinished();
	DWORD SendProcessNameAndPID();
	void AddEntriesInReportsDB(CString eScanType, CString csThreatName, CString csFilePath, CString csAction);
	void InsertItem(CString csFirstParam, CString csSecondParam,CString csThirdParam, CString csForthParam);
	bool ShutDownRootkitCleaning();
	void AddDetectedCount(DWORD dwDetectedEntries);
	void AddTotalCount(DWORD dwTotalEntries);
	void ShowListcontrolOnBasisofSelection();
	bool RootKitScanningStarted();
	bool OnBnClickBtnFirstAntirootkitUI();
	//In antirootkit scan if we abort the scan,files scanned count is shown "0".
	//Niranjan Deshak - 05/03/2015.
	void SetRootkitTotalNDetectedCount(DWORD dwTotalFileCnt, DWORD dwTotalDriveCnt, DWORD dwTotalProcCnt, DWORD dwDetectedFileCnt, DWORD dwDetectedDriveCnt, DWORD dwDetectedProcCnt);
	
	afx_msg void OnNMCustomdrawListAntirootkit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawListRegistry(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawListFilesfolders(NMHDR *pNMHDR, LRESULT *pResult);
	CColorStatic m_stAntirootkitHeader;
	CColorStatic m_stAntirootkitSubHeader;
	void RefreshStrings();
	CColorStatic m_csScannedText;
	CColorStatic m_csScannedSubText;
	CColorStatic m_stRegisteryText;
	CColorStatic m_stRegistrySubText;
	CColorStatic m_stFileText;
	CColorStatic m_stFilesSubText;
	void OnBnClickedButtonAntirootkit();
	CButton m_chkWindowRegistry;
	CxSkinButton m_btnBackButton;
	CButton m_chkFileFolder;
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedCheckForWindowregistry();
	afx_msg void OnBnClickedCheckFilefolder();
	bool OnBnClickBtnAntirootkitUI();
	bool RootKitUIDispalyedOnbasisOfFlags();
	afx_msg void OnPaint();
// issue number 23 resolved by lalit In antirootkit scan,position of pause and stop button needs to be change.
	afx_msg void OnBnClickedBtnStop();
	
};

