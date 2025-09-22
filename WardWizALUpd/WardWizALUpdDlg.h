// WardWizALUpdDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include<WinTrust.h>

// CWardWizALUpdDlg dialog
class CWardWizALUpdDlg : public CDialog
{
// Construction
public:
	CWardWizALUpdDlg(CWnd* pParent = NULL);	// standard constructor
	
// Dialog Data
	enum { IDD = IDD_WARDWIZALUPD_DIALOG };
	int AddSignToSpecifiedFile(CString fPath, bool bUpdateVersion = true);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions

	int m_nCurHeight;
	int m_nScrollPos;
	CRect m_rect;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	bool IsFileHaveSignature(CString FileName);
	bool AddSignatureToFinalSetup(void);
	bool UploadLogFilesToServer(TCHAR *szDestDirPath,bool isbErrorOccur);
	bool mbIsFileSelected;
	CString msSelectedFilePath;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CButton m_ProdType_Essential;
	CButton m_ProdType_Pro;
	CButton m_ProdType_Elite;
	CButton m_OSVersion_32;
	CButton m_OSVersion_64;
	CButton m_MakeSetup;
	CButton	m_PatchType_Product;
	CButton	m_PatchType_Virus;
	CListCtrl m_List_PatchFiles;

	TCHAR	m_szApplPath[512] ;
	TCHAR	m_szOutDirName[128] ;

	TCHAR	m_szProgDir[128] ;
	TCHAR	m_szProgDirX64[128] ;

	TCHAR	m_szWinDir[256];

	TCHAR	m_szWWBinariesPath[256];
	TCHAR	m_szCurrProdVersion[256];

	TCHAR	m_szBasicIniPath[512];
	TCHAR	m_szProIniPath[512];
	TCHAR	m_szEliteIniPath[512];
	TCHAR	m_szEssentialIniPath[512];
	TCHAR	m_szEssentialPlusIniPath[512];

	//For Testing NiranjanD.
	TCHAR	m_szWWizBasicIniPath[512];
	TCHAR	m_szWWizEssentialIniPath[512];
	TCHAR	m_szWWizEssentialPlusIniPath[512];
	TCHAR   m_szWWizEliteIniPath[512];
	TCHAR   m_szWWizEliteServerIniPath[512];

	TCHAR	m_szStatusText[512];
	TCHAR	m_szBasicCount[256];
	TCHAR	m_szEssentialCount[256];
	TCHAR	m_szEssentialPlusCount[256];
	TCHAR	m_szProCount[256];
	TCHAR	m_szEliteCount[256];
	TCHAR	m_szEliteServerCount[256];
	TCHAR	m_szSHA1Count[256];
	TCHAR	m_szSHA2Count[256];
	
	TCHAR   m_szRegOptCount[256];

	TCHAR	m_szProdVersion[256];
	TCHAR	m_szProductVersion[256];
	TCHAR	m_szDatabaseVersion[256];
	TCHAR	m_szScanEngineVersion[256];
	TCHAR	m_szEncVersion[256];
	TCHAR	m_szPatchFolderPath[512];
	CString msSelectedFiles[512];
	CString msSelectedFileForMD5;

	int  m_iBasicCount;
	int  m_iEssentialCount;
	int  m_iEssentialPlusCount;
	int  m_iProCount;
	int  m_iEliteCount;
	int  m_iEliteServerCount;
	int  m_iRegOptCount;
	int	 m_iSHA1Count;
	int  m_iSHA2Count;
	

	bool m_bIsSetupCreated;
	bool m_bIsSetupCreatedAndFilesUploaded;
	int m_iSelectedFileCount;
//	CStringArray m_arBasicArr;
	CList<CString,CString&> m_listBasic;
	CList<CString,CString&> m_listEssential;
	CList<CString,CString&> m_listPro;
	CList<CString,CString&> m_listElite;
	CList<CString, CString&> m_listEliteServer;
	CList<CString,CString&> m_listEssentialPlus;
	CList<CString,CString&> m_listRegOpt;
	std::vector<CString>   m_listOfWWBinaryKeys;
	
	std::vector<CString> m_listSHA1SignBinaries;
	std::vector<CString> m_listSHA2SignBinaries;
		
	DWORD	m_FilesCount;
	DWORD	m_ListItemCount;

	HANDLE	m_hThread_GenerateUpdateFiles ;
	HANDLE	m_hThread_SetStatus;
	HANDLE	m_hThread_SignStatus;
	BOOL	m_bIsWow64;
	bool	m_bVistaOnward;
	bool    m_bIsSigningCompleted;

	HMODULE	m_hZipDLL;
	HMODULE	m_hHashDLL;


	void IsWow64() ;
	void MakeRequiredFolderStructure( );
	DWORD CreateOtherDirectory( TCHAR *pDirName );
	bool LoadRequiredDLLs();
	bool EnumAndMakeZipFiles(TCHAR *pSour, TCHAR *pDest, TCHAR *pSecName, TCHAR *pShortPath);
	bool GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash);
	DWORD WriteToIni(TCHAR *pKey, TCHAR *pValue, TCHAR *pData, TCHAR *pFileName);
	DWORD WriteToEPSServerIni(TCHAR *pKey, TCHAR *pValue, TCHAR *pData, TCHAR *pFileName);

	DWORD RebuildAllVCProject(CString csConfigurationType, bool bProTicked);
	BOOL RebuildSolution( LPTSTR pszCmdLine );
	bool ParserebuildLog( LPTSTR pszLogPath, char * pszSearchKey );
	bool CheckFileExistenceInSelectedVersion(LPTSTR lpIniFilePath);
	bool LoadBinariesNameAsPerSettings(LPTSTR lpIniFilePath);

	void CreateZip32(LPVOID lpParam);
	void CreateZip64(LPVOID lpParam);
	void CreateZipForDB(LPVOID lpParam);
	void CreateZipForHelpFiles(LPVOID lpParam);
	void CreateZipForEmailScanFiles(LPVOID lpParam);
	void CreateZipForWaveFiles(LPVOID lpParam);
	void CreateZipForLogoFiles(LPVOID lpParam);
	void CreateZipForSecNewsFiles(LPVOID lpParam);
	void CreateZipForSetting(LPVOID lpParam);
	void CreateZipForDrivers32(LPVOID lpParam);
	void CreateZipForDrivers64(LPVOID lpParam);

	void UpdateWWBinary(LPVOID lpParam);
	void WriteVersions(LPVOID lpParam);
	bool UploadFilesToLocalServer();
	void EnumFilesAndCopyToServer( TCHAR *pSource, TCHAR*pDest);

	afx_msg void OnBnClickedButtonGenerateUpdateFiles();
	CButton m_Generate_Update_Files;
	CStatic m_Status;
	CButton m_Close;
	afx_msg void OnBnClickedButtonClose();
	CButton m_RebuildAll;
	CButton m_Rebuild32;
	CButton m_Rebuild64;

	CButton m_rdbRAR;
	CButton m_rdbZIP;

	CButton m_btnBuild;
	CButton m_btnReBuild;
	CButton m_chkBuildType;
	CStatic m_grpBxBuidRebuild;
	CButton m_ProdType_Basic;
	afx_msg void OnBnClickedCheckBuildType();
	CButton m_chkPatchType;
	afx_msg void OnBnClickedCheckCheckPatchType();
	CStatic m_stProdVersion;
	CStatic m_stDatabaseVersion;
	CEdit m_edtProdVersion;
	CEdit m_edtDatabaseVersion;
	afx_msg void OnBnClickedRadioPatchtypeVirus();
	afx_msg void OnBnClickedButtonAbort();
	CEdit mSelectedFileName;
	afx_msg void OnBnClickedButtonAddsign();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_chkwithClam;
	CButton m_chkNoClam;
	CButton m_chkpdb;
	CListCtrl m_lstSelectedFileList;
	//DWORD WINAPI Thread_SetStatus( LPVOID lpParam )
	CButton m_chkOfflinePatches;
	afx_msg void OnBnClickedButtonBrowsefiletomd5();
	afx_msg void OnBnClickedButtonGeneratemd5();
	//CStatic m_stSelectedFilePathForZIP;
	CEdit m_cedtMd5OfSelectedFile;
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CEdit m_stSelectedFilePathForZIP;
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	CString m_csRegOptStatus;//This string is for status e.g add/modify/del
	int m_iRadioROAdd;
	afx_msg void OnBnClickedRadioAdd();
	CButton m_btnAdd;
	afx_msg void OnBnClickedRadioModify();
	afx_msg void OnBnClickedRadioDelete();
	CButton m_chkRegOpt;//for checkbox of registry option.
	afx_msg void OnBnClickedCheckRegopt();
	// //This button is used to add registry options selected by user to listcontrol
	CButton m_btnROAddToList;
	// This button is used to remove the selected entry from list control in registry option.
	CButton m_btnRORemoveFromList;
	afx_msg void OnBnClickedButtonRoAdd();//To add registry options to list control
	afx_msg void OnBnClickedButtonRemove();
	CComboBox m_btnRORegKeyType;//For add or select registry Key from combo box
	CString m_csRORegKeyName;//String For resisrty key option e.g = HKLM
	CEdit m_editROvalue;
	TCHAR m_szROValue[MAX_PATH];
	CEdit m_editROPath;
	TCHAR m_szROPath[MAX_PATH];
	CListCtrl m_List_RegOpt;
	int m_iListROCount;
	void AddRegOptToListFromWWBinary();
	bool ParseVersionString(int iDigits[4], CString& csVersion);
	bool zip(CString csFilePath, CString csZipFilePath, const char * ptrZipPath);
	bool GetInnoAndAppPath(CString &csInnoPath, CString &csISSPath);
	DWORD WriteKeyValueToIni(TCHAR *pKey, TCHAR *pValue, TCHAR *pData, TCHAR *WWpFileName, TCHAR *WWizpFileName);
	DWORD GetKeyValueFromIni(TCHAR *pkeyName);

	bool IsFileSetupDllFile(CString csFilePath);

	void CreateZipForRarFiles(LPVOID lpParam);
	CButton m_chkUtility;

	bool CreateAndCopyFilesforBasicEssPro(CString csDestPathSetupDir, CString m_szPatchFolderPath);
	bool CreateFolderStructureForSetups(CString csDestPathSetupDir);
	void CreateZipForSqliteDLL(LPVOID lpParam);
	void CreateZipForSciterDLL(LPVOID lpParam);
	void CreateZipForNSSFolder(LPVOID lpParam);
	void CreateZipForSETUPDBFolder(LPVOID lpParam);
	bool RarFile(CString csFilePath, CString csRarFilePath, const char * ptrRarFilePath);

	TCHAR		m_csDestPathSetupDirBasic[512];
	TCHAR		m_csDestPathSetupDirBasicClam[512];
	TCHAR		m_csDestPathSetupDirBasicNoClam[512];
	TCHAR		m_csDestPathSetupDirBasicOffline[512];

	TCHAR		m_csDestPathSetupDirEssential[512];
	TCHAR		m_csDestPathSetupDirEssentialClam[512] ;
	TCHAR		m_csDestPathSetupDirEssentialNoClam[512] ;
	TCHAR		m_csDestPathSetupDirEssentialOffline[512] ;

	TCHAR		m_csDestPathSetupDirPro[512] ;
	TCHAR		m_csDestPathSetupDirProClam[512];
	TCHAR		m_csDestPathSetupDirProNoClam[512];
	TCHAR		m_csDestPathSetupDirProOffline[512];

	TCHAR		m_csDestPathSetupDirElite[512];
	TCHAR		m_csDestPathSetupDirEliteClam[512];
	TCHAR		m_csDestPathSetupDirEliteNoClam[512];
	TCHAR		m_csDestPathSetupDirEliteOffline[512];

	TCHAR		m_csDestPathSetupDirEssentialPlus[512];
	TCHAR		m_csDestPathSetupDirEssentialPlusClam[512];
	TCHAR		m_csDestPathSetupDirEssentialPlusNoClam[512];
	TCHAR		m_csDestPathSetupDirEssentialPlusOffline[512];
	
	CEdit m_edtScanEngineVersion;
	CButton m_ProdType_EssPlus;
};
