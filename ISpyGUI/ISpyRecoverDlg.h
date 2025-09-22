#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "PictureEx.h"
#include "afxcmn.h"
#include "ISpyDataManager.h" 


// CISpyRecoverDlg dialog

class CISpyRecoverDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyRecoverDlg)

public:
	CISpyRecoverDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyRecoverDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_RECOVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	LRESULT OnNcHitTest(CPoint point);

	TCHAR						m_szQuarantineFilePath[MAX_PATH];
	TCHAR						m_szOriginalThreatPath[MAX_PATH];
	HCURSOR						m_hButtonCursor;
	CStatic						m_stRecoverHeaderpic;
	HBITMAP						m_bmpRecoverHeaderpic;
	CListCtrl					m_lstRecover;
	CxSkinButton				m_btnRecover;
	CxSkinButton				m_btnBack;
	CButton						m_chkSelectAll;
	CColorStatic				m_stSelectAll;
	CDataManager				m_objRecoverdb;
	CDataManager				m_objRecoverdbToSave;
	DWORD						m_dwEnKey ;
	int							m_iTotalEntriesCount;
	HANDLE						m_hThread_Recover;
	HANDLE						m_hThread_Delete;
	bool						m_bRecoverThreadStart;
	bool						m_bDeleteThreadStart;
	bool						m_bRecoverStop;
	bool						m_bDeleteStop;
	bool						m_bRecoverClose;
	bool						m_bRecoverBrowsepath;
	HANDLE						m_hShowRecoverEntriesThread;
	TCHAR						m_szBrowseFilePath[MAX_PATH];
	CxSkinButton				m_btnDelete;
	bool						m_bIsPopUpDisplayed;
	CISpyCommunicator			m_objCom;// (SERVICE_SERVER, false);

	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonRecover();
	CString GetQuarantineFolderPath();
	bool isAnyEntrySeletected();
	void PopulateList(bool bCheckEntry = false);
	void LoadExistingRecoverFile(bool bType);
	bool LoadDataContentFromFile(CString csPathName);
	afx_msg void OnBnClickedCheckSelectall();
	bool RecoverFile(CString ThreatPath, CString VirusName);
	DWORD Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus);
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	void StoreExistingRecoverFile();
	bool StoredataContentToFile(CString csPathName);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	bool CreateFullDirectoryPath(wchar_t *szFullPath);
	bool SendRecoverOperations2Service(int iMessageInfo, CString csRecoverFileEntry ,CString csBrowseRecoverFileEntry, DWORD dwType,DWORD dwTypeofAction, bool bWait = false);
	bool BrowseFileToSaveRecover(CString &csBroswseFilepath);
	CColorStatic m_stRecoverHeaderName;
	CColorStatic m_stRecoverDesc;

	void RecoverEntries();
	void DeleteREntries();
	bool StopReportOperation();
	void RefreshStrings();
	void OnBnClickedRecover();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnPaint();
//Issue No.17 While recover and deletion is in progress if I click Recover button then Recover does not takes place
	// Gaurav Chopdar Date:6/1/2015
     bool m_bRecoverEnableDisable;

	 //Added by Vilas on 27 March 2015
	 //To handle file is in use 
	 bool SendRecoverOperations2Service(ISPY_PIPE_DATA *pszPipeData, bool bWait);
	 bool GetSortedFileNames(vector<int> &vec);
	 void GetFileDigits(LPCTSTR pstr, vector<int> &vec);
	 void LoadRemainingEntries();
	 bool SaveDBFile();
};
