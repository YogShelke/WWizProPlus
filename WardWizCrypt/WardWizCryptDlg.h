/**********************************************************************************************************
	Program Name			: WardWizCryptDlg.cpp
	Description				: This class is derived from CDialogEx, which is main Window for this project
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 5th Jun 2015
	Version No				: 1.11.0.0
	Special Logic Used		:
	Modification Log		:
***********************************************************************************************************/
#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ZipArchive.h"
#include "WardWizCompressor.h"
#include "ISpyCommServer.h"
#include "PipeConstants.h"
#include "ISpyCommunicator.h"
#include "WardWizCrypter.h"
#include <vector>
// CWardWizCryptDlg dialog
class CWardWizCryptDlg : public CDialogEx
{
// Construction
public:
	CWardWizCryptDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WARDWIZCRYPT_DIALOG };

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
	afx_msg void OnBnClickedButtonBrowseFolder();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonCancel();
	bool EncryptOperation();
	bool DecryptOperation();
	bool UpdateResults();
	void InitializeVariables();
	void UpdateStatus(CString csStatus);
	static void UpdateOpProcess(DWORD dwpercent, void * param);
	bool EncryptSingleFile(CString csFilePath, CString csOutputPath, CString csOrgFileName);
	bool DecryptSingleFile(CString csFilePath, CString csTmpPath, bool bTempRequired);
	static void OnDataReceiveCallBack(LPVOID lpParam);
	bool SendInfo2UI(int iRequest, CString csFilePath, DWORD dwFileStatus, DWORD  dwNewInsertItem = 1);
	void SendStatustoUI(DWORD dwRet, CString szInputFilePath);
	void CryptOprFinished();
	void EndingLogEntry();
	void StartUpLogEntry();
	bool GetSelectedFileSize(TCHAR *pFilePath, DWORD64 &dwFileSize);
	bool IsDriveHaveRequiredSpace(TCHAR szDrive, CString csFilePath, int iSpaceRatio);
	DWORD IsFileAlreadyEncrypted(CString csFilePath);
	DWORD64 GetTotalNumberOfFreeBytes(void);
	TCHAR GetDefaultDrive(CString csFilePath);
	bool  IsFileValidForDecompress(CString csOrgFileNamePath);
	DWORD EnumerateAllFolderAndFiles(CString csaTokenEntryFilePath[]);
	void  SendCompressionAbortMsgToGUI();
	bool  IsPathBelongToSameCurntDirectory(CString cs_oldPath, CString cs_newPath);
	static void  FileSaveAs(LPTSTR lpFilePath, void * param);
	DWORD  IsFileSizeMorethen3G(CString csFilePath);
	void CryptOprFailed();
	bool FileIntegrityRollBack();
	bool AddFileInfoToINI(CString csfilePath, DWORD dwCRC, DWORD dwAttemp);
	void GetEnviormentVariablesForAllMachine();
	bool IsPathBelongsToOSReservDirectory(CString folderPath);
	bool IsWardwizFile(CString csFilefolderPath);
	bool GetSystemTempPath(CString csFileFolderPath, CString &csOutTmpPath);
	bool IsSizeForTempAvble();
	void GetAllSystemDrive();
	void SendLowDiskSpaceMessageToGUI();
	bool IsPathBelongsToWardWizDir(CString csFilefolderPath);
	CString ExpandShortcut(CString& csFilename);
public:
	CEdit							m_editStatus;
	CEdit							m_editFolderPath;
	CButton							m_btnBrowse;
	CButton							m_btnStart;
	CButton							m_btnCancel;
	CStatic							m_stFilesProcessed;
	CStatic							m_stTotalFiles;
	CListCtrl						m_lstCryptReport;
	CButton							m_rdbEncrypt;
	CButton							m_rdbFile;
	HANDLE							m_hThreadCrypto;
	HANDLE							m_hThreadTimer;
	CString							m_csFileFolderPath;
	CString							m_csPassword;
	CEdit							m_editPassword;
	BOOL							m_bIsEncrypt;
	BOOL							m_bIsFile;
	DWORD							m_dwFilesProcessed;
	DWORD							m_dwTotalFiles;
	CStatic							m_stTotalFilesCount;
	CTime							m_tsStartTime;
	DWORD							m_dwKeepOriginal;
	CISpyCommunicator				m_objCom;
	static CWardWizCryptDlg	 		*m_pThis;
	CWardWizCrypter					m_objWardWizCrypter;
	TCHAR							m_szOutputFilePath[MAX_PATH];
	TCHAR							m_szInputFilePath[MAX_PATH];
	HANDLE							m_hEncrDecryptionThread;
	HANDLE							m_hEvtStop;
	CString							m_csSelectedFilePath;
	CTime							m_tsCmpDmpStartTime;
	HANDLE							m_LogMutex;
	bool							m_IsOperSuccess;
	CString							m_csEmptyDrive;
	ULARGE_INTEGER					m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
	ULARGE_INTEGER					m_uliTotalNumberOfBytes;     // bytes no disco
	ULARGE_INTEGER					m_uliTotalNumberOfFreeBytes; // bytes livres no disco
	CStringArray					m_csListOfFileToDlt;
	CString							m_csOrgFileName;			// to keep org. file name of file to be ecrypted.
	CString							m_csOrgFileNamePath;
	CWardWizCompressor				m_CWardWizCompressor;
	//CString						m_csSelectedFilePath;
	CString							m_csCurntArchTmpFilePath;
	CString							m_csCurntEncFilePath;
	CString							m_csFinalOutputFilePath;
	bool							m_bIsAborted;
	bool							m_bIsEnDecStarted;
	bool							m_bIsComDomStarted;
	CString							m_csSaveAsPath;
	bool							m_bIsManualAbort;
	CString							m_csProgramData;
	CString							m_csWindow;
	CString							m_csProgramFile;
	CString							m_csProgramFilex86;
	CString							m_csAppData4;
	CString							m_csDriveName;
	CString							m_csArchitecture;
	CString							m_csBrowseFilePath;
	CString							m_csUserProfile;
	CString							m_csAppData;
	CString							m_csModulePath;
	bool							m_bIsFileFromOSPath;
	std::vector<CString>			m_vcsListOfFilePath;
	DWORD64							m_dwRequiredSpace;
	std::vector<TCHAR>				m_vcsListOfDrive;
	HANDLE						    m_hStopEvent;

	typedef enum {EQUAL_SIZE = 1, DOUBLE_SIZE, TRIPLE_SIZE } DISK_SIZE;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()	
};
