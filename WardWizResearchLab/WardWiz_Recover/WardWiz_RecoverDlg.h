
// WardWiz_RecoverDlg.h : header file
//
#pragma once
#include "afxcmn.h"


const unsigned int		WRDWIZ_KEY_SIZE = 0x10;
const unsigned int		WRDWIZ_SIG_SIZE = 0x07;
const unsigned char		WRDWIZ_SIG[WRDWIZ_SIG_SIZE + 1] = "WARDWIZ";

// CWardWiz_RecoverDlg dialog
class CWardWiz_RecoverDlg : public CDialogEx
{
// Construction
public:
	CWardWiz_RecoverDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WARDWIZ_RECOVER_DIALOG };

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
	afx_msg void OnBnClickRecover();
	CListCtrl m_lstFoundEntries;
	CString m_szFilePath;

	afx_msg void OnBnClickBrowse();
	//afx_msg void OnEnChangeEdit1();

	DWORD Decrypt_File(TCHAR* szRecoverFilePath, TCHAR* szOriginalThreatPath, DWORD &dwStatus, bool bDeleteBackupFile);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	bool ReadKeyFromEncryptedFile(HANDLE hFile);
	bool IsFileAlreadyEncrypted(HANDLE hFile);

public:

	CEdit					m_editFilePath;
	CEdit					m_editStatus;
	CStatic					m_RecoverFileCount;
	TCHAR					m_szOriginalThreatPath[260];
	unsigned char			*m_pbyEncDecKey;
	bool					m_bIsRecoverStart;

	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickCancel();
	CString m_RecoverStatus;
	CString m_RecFileCount;
};

typedef struct
{
	int		iMessageInfo;						//; Message ID
	//HKEY	hHey;								//; Used to send hive
	BYTE	byData[1000];						//; byte data used for Registy type SZ_BINARY
	DWORD	dwValue;							//; Used for Spy ID
	DWORD	dwSecondValue;						//; Used for Spy ID
	TCHAR	szFirstParam[1000];					//; Used for VIRUS_PATH
	TCHAR	szSecondParam[500];					//; Used for VIRUS_NAME
	TCHAR	szThirdParam[500];					//; Used to send REGISTRY VALUEs
}ISPY_PIPE_DATA, *LPISPY_PIPE_DATA;

enum
{
	RECOVER_FILE	=	1010
};