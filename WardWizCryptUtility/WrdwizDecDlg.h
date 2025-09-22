#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "ColorStatic.h"
#include "ColorEdit.h"
#include "xSkinButton.h"

// CWrdwizDecDlg dialog

class CWrdwizDecDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CWrdwizDecDlg)

public:
	CWrdwizDecDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWrdwizDecDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DECRYPT_OLD_FILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CRect					m_rect;
	CColorEdit					m_edtSelectFile;
	CButton					m_btnBrowse;
	CString					m_csFilePath;
	HANDLE					m_hEncDecThread;
	unsigned char			*m_pbyEncDecKey;
	unsigned char			*m_pbyEncDecSig;
	CButton					m_btnStart;
	CButton					m_btnCancel;
	CColorEdit				m_edtFileStatus;
	CColorStatic			m_stDecryptToolDesc;
	CColorStatic			m_stDecryptToolHeader;
	bool					m_bIsEnDepInProgress;
	CxSkinButton			m_btnClose;
	CColorStatic			m_stAllRightsReserved;

	

	DWORD Decrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus);
	DWORD ReadKeyFromEncryptedFile(HANDLE hFile);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	DWORD IsFileAlreadyEncrypted(HANDLE hFile);


	
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonCancel();
};
