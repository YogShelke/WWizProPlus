#pragma once
#include "afxwin.h"
#include "ColorStatic.h"
#include "afxcmn.h"
#include "SpywareFoundDlg.h"
#include "xSkinButton.h"
#include "DrivePickerListCtrl.h"
#include "TextProgressCtrl.h"
#include "ColorEdit.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "ScannerContants.h"
#include "ISpyDataManager.h"
#include "iSpyMemMapClient.h"
#include "CFileDropCStatic.h"
#include "StaticFilespec.h"

class CScanDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CScanDlg)
private:
	CScanDlg(CWnd* pParent = NULL);   // standard constructor
public:
	static CScanDlg* GetScanDlgInstance(CWnd* pParent);
	static void ResetInstance();
	virtual ~CScanDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//afx_msg void OnBnClickedBackButton();
public:
	virtual BOOL OnInitDialog();
	void AddEntriesInReportsDB(CString csDate,CTime ctDateTime,SCANTYPE csScanType,CString csThreatName,CString csFilePath,CString csAction);
	//bool SaveInReportsDB();
	//void LoadReportsDBFile();
	bool ISDuplicateEntry(CString strScanFileName);
	void InsertItem(CString strVirusName, CString strScanFileName, CString csAction, CString csISpyID);
	void StartScanning();
	void ReInitializeBeforeStartScan();
	void EnumFolder(LPCTSTR szFolderPath);
	void SetScanStatus(CString csStatus);
	void EnableAllWindows(BOOL bEnable);
	void TextSetWindow(CString csStatus);
	void WriteStdOut(LPTSTR msg, BOOL freshclam);
	BOOL LaunchClamAV(LPTSTR pszCmdLine, HANDLE hStdOut, HANDLE hStdErr);
	void RedirectStdOutput(BOOL freshclam);
	bool GetVirusNameAndPath(CString csStatus, CString &csVirusName, CString &csVirusPath);
	CString GetFileNameOnly(CString csInputPath);
	bool ShowSpyFoundDlg(bool bCloseUI = false);
	bool IsFullString(CString csInputPath);
	void QuaratineEntries();
	CString GetQuarantineFolderPath();
	bool QuarantineFile(CString csFilePath);
	bool GetFileName(CString csFilePath, CString &csFileName);
	void ReInitializeVariables();
	bool ShutDownScanning(bool bCloseUI = false);
	bool bisLastScan;
	bool GetAllDrivesList(CStringArray &csaReturn);
	void SetScanPath(CString csScanPath);
	bool GetScanningPaths(CStringArray &csaReturn);
	void StartUSBScan();
	bool OnGetSelection();
	void SelectDriveParameter(LPCTSTR cs);
	void SetControls4USBDetect();
	void GetModuleCount() ;
	void ResetControlsValues();	
	void SetReportDate();
	bool ISAllItemsCleaned();
	bool m_bIsCleaning;
	//bool LoadRequiredModules();
	//bool LoadSignatures(LPTSTR lpFilePath);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedButtonClean();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	bool OnBnClickedBtnQuickscan();
	bool OnBnClickedBtnFullscan();
	bool OnBnClickedBtnCustomscan();
	afx_msg void OnBnClickedButtonStartScan();
	afx_msg void OnBnClickedButtonCustomscan();
	afx_msg void OnBnClickedButtonBrowsepath();
	afx_msg void OnNMCustomdrawListViruslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckSelectall();
	afx_msg void OnBnClickedCheckDisablesound();

	bool ReadUISettingFromRegistry();
	void MakeEntryInRegistry();
	DWORD Encrypt_File( TCHAR *m_szFilePath, DWORD &dwStatus ) ;
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	BOOL BackUpBeforeQuarantineOrRepair(CString csOriginalThreatPath);

	//void RecoverEntries(CString strOriginalName,CString strDuplicateName,CString strThreatName);
	//BOOL StoredataContentToFile(CString csPathName);
	//void LoadRecoverDBFile();

	bool StartScanUsingService(CStringArray &csaAllScanPaths);
	bool SendRequestCommon(int iRequest);
	bool MakeFullTokenizedScanPath(CStringArray &csaAllScanPaths, LPTSTR szScanPath);
	bool ScanningFinished();
	bool ScanningStarted();
	void ShowStaus(LPCTSTR szStatus);
	void ShowVirusEntry(LPCTSTR szStatus);
	void SetTotalFileCount(DWORD dwFileCount, DWORD dwIsMemScan = FALSE);
	bool SendFile4RepairUsingService(int iMessage, CString csThreatPath,CString csThreatName, DWORD dwISpyID, bool bWait = false, bool bReconnect = false);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait = false);
	bool SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry ,DWORD dwType, bool bWait = false, bool bReconnect = false);
	bool Check4DBFiles();
	void EnableDisableControlForClean(bool bEnable);
	bool CloseCleaning();
	bool ThreatsFound();
	afx_msg void OnPaint();
	bool RefreshStrings();
	bool SaveDBEntries();
	//Updated By Nitin K. 19th March 2015
	bool EnumFolderToCheckEmptyFolder(LPCTSTR szFolderPath);
	bool VirusFoundEntries(LPISPY_PIPE_DATA lpSpyData);
	void GetShortFilePath(CString csFilePath);
	DWORD CheckForDeleteFileINIEntries();
	bool Check4ValidDBFiles(CString csDBFilePath);

	void HideControlForCustomScan(bool bFlag);
	void showListControlForCustomScan();
	CImageList m_ImageList;
	void ShowControlsAfterCustomScanStarted(bool bFlag);
	CString ExpandShortcut(CString& csFilename) const;
	void AddDeleteEditCustomScanEntryToINI();
	DWORD					m_dwFailedDeleteFileCount;
	void LoadCutomScanINI();
	TCHAR	m_szCustomCount[256];
	int m_iCustomCount;
	CStringArray m_listCustomCount;

	//added by Vilas on 07 April 2015 to handle failure cases
	bool SendFile4RepairUsingService(ISPY_PIPE_DATA *pszPipeData, bool bWait, bool bReconnect);
	DWORD CheckForRepairFileINIEntries();
	bool PauseScan();
	bool ResumeScan();
	bool MemScanningFinished();
	bool SaveLocalDatabase();
public:
	bool					m_bQuickScan;
	bool					m_bFullScan;
	bool					m_bCustomscan;
	int						m_nContactVersion;
	DWORD					m_dwEnKey ;
	TCHAR					m_szAppData[512];
	TCHAR					m_szEncFile[1024];
	TCHAR					m_szOriginalFilePath[512];
	bool					m_ScanCount ;
	bool					bVirusFound;
	bool					m_bScnAborted;
	bool					m_bPlaySound;
	CTextProgressCtrl		m_prgScanProgress;
	CColorStatic			m_ScanPercentage;
	CColorStatic			m_stTitle;
	CColorStatic			m_csFilesScanned;
	CColorStatic			m_stElapsedTime;
	CColorStatic			m_stThreatsFound;
	CStaticFilespec			m_edtStatus;
	CListCtrl				m_lstVirusesList;
	CStringArray			m_csaAllScanPaths;
	SCANTYPE				m_eScanType;
	SCANTYPE				m_eCurrentSelectedScanType;
	//CDataManager			m_objRecoverEntriesdb;
	CString					m_csDuplicateName;	
/*	int						m_virusFound;	
	int						m_FileScanned;
	int						m_iTotalFileCount;*/
	DWORD					m_virusFound;	
	DWORD					m_FileScanned;
	DWORD					m_iTotalFileCount;
	DWORD					m_iMemScanTotalFileCount;
	bool					m_bRedFlag;
	CTime					m_tsScanStartTime;
	CTime					m_tsScanEndTime;
	CTime					m_tsScanPauseResumeTime;
	CTimeSpan				m_tsScanPauseResumeElapsedTime;
	bool					m_bScanStarted;
	bool					m_bScanningStopped;
	bool					m_bQuarantineFinished;
	HCURSOR					m_hButtonCursor;
	CxSkinButton			m_btnStop;
	CxSkinButton			m_btnClean;
	CxSkinButton			m_btninnerCutomScan;
	CxSkinButton			m_btnPauseResume;
	CxSkinButton			m_btnScan;
	CxSkinButton			m_btnBrowse;
	CEdit					m_EdtInputPath;
	HBITMAP				    m_bmpQuickScan;
	CBitmap				    m_bmpFullScan;
	CBitmap				    m_bmpCustomScan;
	CBitmap				    m_bmpUsbScan;
	CBitmap				    m_bmpRemovableDevices;
	CStatic                 m_stHeaderAll;
	CStatic					m_stHeaderRemovable;
	DWORD				    m_dwTotalScanPathCount;
	DWORD					m_dwTotalRemain;
	CListCtrl				m_lstDrivePicker;
	CButton 				m_chkSelectAll;
	CColorStatic			m_stSelectAll;
	HMODULE					m_hModuleISpyScanDLL;
	HMODULE                 m_hModuleISpyRepairDLL;
	CButton 				m_chkDisableSound;
	CColorStatic			m_stDisableSound;
	int						m_iContactVersion;
	bool					m_bSuccess;
	HANDLE					m_hScanStopEvent;
	HANDLE					m_hThreadVirEntries;
	HANDLE					m_hThreadStatusEntry;
	CString					m_csPreviousFile;
	iSpyServerMemMap_Client m_objIPCClient;
	iSpyServerMemMap_Client m_objIPCClientVirusFound;
	bool					m_bClose;
	bool					m_bHome;
	CColorStatic			m_stQuickHeaderName;
	CColorStatic			m_stQuickHeaderDescription;
	CColorStatic			m_stFullScanHeaderName;
	CColorStatic			m_stFullScanHeaderDes;
	CColorStatic			m_stCustomScnHeaderName;
	CColorStatic			m_stCustomHeaderDes;
	CTimeSpan				m_tsScanElapsedTime;
	HANDLE					m_hQuarantineThread;
	CISpyCommunicator		m_objCom;
	bool					m_bScanStartedStatusOnGUI;
	bool					m_bFlagScanFinished;
	int						m_iTotalNoofFiles;
	bool					m_bSignatureFailed; //Varada Ikhar
	TCHAR					m_szShortPath[120];
	bool					m_bOnWMClose;
	bool					m_bIsMemScan;
private:
	static bool				m_instanceFlag;
	static CScanDlg			*m_pobjScanDlg;
	DECLARE_MESSAGE_MAP()
public:
	CFileDropCStatic		m_stDragDropFiles;
	CxSkinButton			m_btnCutomAdd;
	CxSkinButton			m_btnCustomEdit;
	CxSkinButton			m_btnCustomDelete;
	CButton					m_cbCustomSelectAll;
	HBITMAP				    m_bmpDragAndDrop;
	afx_msg void OnBnClickedCheckCustomSelectAll();
	afx_msg void OnBnClickedButtonCustomAdd();
	afx_msg void OnBnClickedButtonCustomEdit();
	afx_msg void OnBnClickedButtonCustomDelete();
	CColorStatic			m_stCustomDragDropText;
	CString					m_csStaticElapsedTime;
	CString					m_csStaticFilesScanned;
	CString					m_csStaticThreatsFound;
	bool					m_bIsPopUpDisplayed;
};
