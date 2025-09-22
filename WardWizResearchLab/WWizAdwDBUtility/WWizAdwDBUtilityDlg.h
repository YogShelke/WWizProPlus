
// WWizAdwDBUtilityDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <algorithm>
#include "AdwCleanerConstants.h"

typedef enum FUNCTIONS
{
	NOSELECTION,
	SERVICES,
	FOLDERS,
	FILES,
	SHORTCUTS,
	TASKS,
	REGISTRY,
	BROWSERSFILES,
	BROWSERSREGISTRY
};

const unsigned int MAX_KEY_SIZE = 0x10;
const unsigned int MAX_SIG_SIZE = 0x08;

const unsigned int MAX_VERSIONCHKBUFF = 0x50;
const unsigned int MAX_TOKENSTRINGLEN = 0x04;

const unsigned int MAX_BUFF_SIZE = 0x10 * 0x1000;
const unsigned int MAX_DBVERSIONNO_SIZE = 0x7;

template <typename T>
void remove_duplicates(std::vector<T>& vec)
{
	std::sort(vec.begin(), vec.end());
	vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

// CWWizAdwDBUtilityDlg dialog
class CWWizAdwDBUtilityDlg : public CDialogEx
{
// Construction
public:
	CWWizAdwDBUtilityDlg(CWnd* pParent = NULL);	// standard constructor
	~CWWizAdwDBUtilityDlg();

// Dialog Data
	enum { IDD = IDD_WWIZADWDBUTILITY_DIALOG };

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
	DECLARE_MESSAGE_MAP()
public:
	CEdit					m_editDBSignature;
	CEdit					m_editVersionNo;
	CEdit					m_editLocFilePath;
	CEdit					m_editDataFile;
	CButton					m_btnBrowseLocFile;
	CButton					m_btnBrowseDataFile;
	CButton					m_btnStart;
	CButton					m_btnCancel;
	CStatic					m_stTotalCount;
	CStatic					m_stNewEntries;
	CEdit					m_editStatus;
	CStatic					m_stTotalLocCount;
	CStatic					m_stTotalNewLocEntries;
	CEdit					m_editExistingDBPath;

	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonLfileBrowse();
	afx_msg void OnBnClickedButtonDfileBrowse();
	afx_msg void OnBnClickedRadioLocations();
	afx_msg void OnBnClickedRadioData();
	afx_msg void OnBnClickedButtonExdbfileBrowse();
	afx_msg void OnBnClickedButtonLoadDb();
	FUNCTIONS CWWizAdwDBUtilityDlg::GetCurrentFunSelection();
	bool MakeBuffer(std::vector<string> &vecData);
	bool SaveDataIntoFile(LPTSTR szFilePath, STRUCTADWCLEANER &szAdwClean, LPSTR szSignature, LPSTR szVersion, DWORD dwVersionLength, ULONG64 ulFileCRC);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	bool LoadContaintFromFile(LPTSTR szFilePath, DWORD dwVersionLength, DWORD &dwSigCount);
	bool IsFileAlreadyEncrypted(CString csFileName, DWORD &dwDBVersionLength, DWORD &dwDBMajorVersion);
	bool ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion);
	void DoReportingStuff();
	void ClearVectorData();
public:
	std::vector<string>		m_vecServicesLocations;
	std::vector<string>		m_vecServicesData;
	std::vector<string>		m_vecFoldersLocations;
	std::vector<string>		m_vecFoldersData;
	std::vector<string>		m_vecFilesLocations;
	std::vector<string>		m_vecFilesData;
	std::vector<string>		m_vecShortcutsLocations;
	std::vector<string>		m_vecShortcutsData;
	std::vector<string>		m_vecsTasksLocations;
	std::vector<string>		m_vecTasksData;
	std::vector<string>		m_vecRegLocations;
	std::vector<string>		m_vecRegData;
	std::vector<string>		m_vecBrowserLocations;
	std::vector<string>		m_vecBrowserData;
	std::vector<string>		m_vecBrowserRegLocations;
	std::vector<string>		m_vecBrowserRegData;
	DWORD					m_dwTotalCount;
	DWORD					m_dwNewlyAddedCount;
	DWORD					m_dwTotalLocCount;
	DWORD					m_dwNewlyAddedLocCount;
	DWORD					m_dwBufSize;
	DWORD					m_dwBufOffset;
	DWORD					m_dwCharLen;
	LPBYTE					m_lpbyBuffer;
	unsigned long			m_ulFileSize;
	unsigned char			*m_pbyEncDecSig;
	STRUCTADWCLEANER		m_stAdwCleaner;
	CStatic m_stServicesL;
	CStatic m_stServicesD;
	CStatic m_stFoldersL;
	CStatic m_stFoldersD;
	CStatic m_stFilesL;
	CStatic m_stFilesD;
	CStatic m_stShortCutL;
	CStatic m_stShortcutD;
	CStatic m_stTasksL;
	CStatic m_stTasksD;
	CStatic m_stRegistryL;
	CStatic m_stRegistryD;
	CStatic m_BrowsersL;
	CStatic m_stBrowsersD;
	CStatic m_stBrowsersRegL;
	CStatic m_stBrowsersRegD;
};
