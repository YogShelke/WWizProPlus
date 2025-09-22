
// WardWizCryptUtilityDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "_fat_aes_cbc.h"


typedef struct _FILECRYPTOPARAM
{
	TCHAR szFileName[MAX_PATH * 2];
	TCHAR szPassword[33];
	TCHAR szMesDigest5[33];
	DWORD dwCryptVersion;

}WRDWIZ_FILECRYPTOPARAM, *PWRDWIZ_FILECRYPTOPARAM;


// CWardWizCryptUtilityDlg dialog
class CWardWizCryptUtilityDlg : public CDialogEx
{
// Construction
public:
	CWardWizCryptUtilityDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WARDWIZCRYPTUTILITY_DIALOG };

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
	CEdit				m_edtFilePath;
	CButton				m_btnBrowse;
	CButton				m_btnStart;
	CListCtrl			m_lstFileDetails;
	CEdit				m_edtPassword;

	CString m_csFilePath;
	DWORD IsFileAlreadyEncrypted(HANDLE hFile);
	DWORD ValidateFile(LPCTSTR m_csFilePath, LPCTSTR lpPass);
	DWORD CheckFileIntegrityBeforeDecryption(LPCTSTR lpFileName);
	DWORD AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd);
	DWORD m_dwFileChecksum;
	DWORD GetDataEncVersion();
	DWORD GetDataEncVersionSEH();
	DWORD ConvertStringTODWORD(TCHAR* szDataEncVersion);
	DWORD	ConvertStringTODWORDSEH(TCHAR* szDataEncVersion);
	bool ParseVersionNoSEH(LPTSTR lpszVersionNo);
	bool ParseVersionNo(LPTSTR lpszVersionNo);
	bool VerifyPassword(LPCTSTR szFilePassword, LPCTSTR szUserPassword);
	bool CovertDataVersionNoDWORDToString();

	DWORD				m_dwDataEncVersionNo;
	DWORD				m_dwDataEncVerfromFile;
	CString				m_csDataEncVer;
	TCHAR				m_szDataEncLogicNo[256];
	TCHAR				m_szDataEncPatchNo[256];
	TCHAR				m_szDataEncVerfromFile[256];

	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonBrowse();
	DECLARE_MESSAGE_MAP()
	
};
