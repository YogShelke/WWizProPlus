
/****************************************************
*  Program Name: DataEncryptionDlg.h                                                                                                   
*  Author Name: Vilas & Prajakta                                                                                                      
*  Date Of Creation: 16 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/

#pragma once
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "ColorEdit.h"
#include "ISpyPasswordPopUpDlg.h"
#include "afxcmn.h"
#include "StaticFilespec.h"
// CDataEncryptionDlg dialog

/****************************************************
CLASS DECLARATION,MEMBER VARIABLE & FUNCTION DECLARATION
****************************************************/

class CDataEncryptionDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CDataEncryptionDlg)

public:
	CDataEncryptionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDataEncryptionDlg();

	enum { IDD = IDD_DIALOG_DATAENCRYPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	virtual BOOL OnInitDialog() ;
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	TCHAR	m_szFilePath[2048] ;
	bool	bEncryption ;
	void InitializeDataEncryptionVaraibles() ;
	void BrowseFolderForFileSelection(TCHAR *pName, TCHAR *pExt ) ;
	bool SendDataEncryptionOperation2Service(const TCHAR * chPipeName, DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, DWORD dwValueOperation, bool bDataEncryptionWait);
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName);
	void ResetControl(bool bEnable);//Nitin K. 
	DWORD Encrypt_File(TCHAR *m_szFilePath, TCHAR *pDestPath, DWORD &dwStatus);
	bool ShutDownEncryptDecrypt(DWORD dwManualStop = 0);
	bool PauseEncryptionDecryption();
	bool ResumeEncryptionDecryption();
	bool IsPathBelongsToWardWizDir(CString csFilefolderPath);
	CString ExpandShortcut(CString& csFilename);
	bool IsPathBelongsFromRemovableDrive(CString csFilefolderPath);
protected:
	HMODULE		m_hEncDecDLL ;
	HMODULE		m_hRegisterDLL ;

public:

	int						m_Check_Encrypt;
	int						m_Check_Decrypt;
	CEdit					m_Edit_FilePath;
	CxSkinButton			m_Button_Browse;
	CxSkinButton			m_Button_Stop;
	CxSkinButton			m_Button_Start;
	CxSkinButton			m_btnEncrypt;
	CxSkinButton			m_btnDecrypt;
	//CxSkinButton			m_btnBack;
	CxSkinButton			m_btnBackArrow;
	//CxSkinButton			m_btnBackDisable;
	DWORD					m_KeyEncrypt;
	HBITMAP					m_bmpDataEncrypt;
	CStatic					m_stHEncrypt;
	CColorStatic			m_stEncrypt;
	CColorStatic			m_stDecrypt;
	CColorStatic			m_stMsg;
	CColorStatic			m_stPreviousFolderLocation;
	HCURSOR					m_hButtonCursor;
	CColorStatic			m_stPrevFileStatus;
	CColorStatic			m_stPrevDecFilePath;
	CxSkinButton			m_btnOpenfolder;
	TCHAR					m_szLastFileEncOrDec[512];
	CColorStatic			m_stHeaderDescription;
	bool					m_bIsEnDepInProgress;
	HANDLE					m_hEncDecThread;
	bool					m_bIsLowDiskSpace;
	CString					m_csMsgforLowDiskSpace;
	bool					m_bIsSystemFolderPathSelected;

	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedRadioEncrypt();
	afx_msg void OnBnClickedRadioDecrypt();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedBtnOpenfolder();
	DWORD FileExists(const TCHAR *fileName);
	afx_msg void OnBnClickedBtnBack();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
//	int m_btnEncrypt;
//	int m_btnDecrypt;
	
	
	CColorStatic m_stEncryptHeaderName;
	CISpyPasswordPopUpDlg m_objPasswordPopUpDlg;

	void RefreshStrings();
	int PopUpDialog();
	DWORD ValidatePassword(CString csPassword);
	bool CheckForWardWizSystemFile(CString scFolderPathTemp, CString csSrcFileName);
	CString GetWardWizSysPathFromRegistry(CString csSubKeyValue, CString csValue);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
public:
	CListCtrl m_lstDataEncDec;
	CStatic m_stGrpBoxDataEncDec;
	CButton m_cbKeepOriginalFile;
	CColorStatic m_stKeepOriginal;
	void StopCryptOperations(DWORD dwSuccessStatus);
	bool m_bManualStop;
	CColorStatic m_stSelectOption;
	CStaticFilespec m_edtCryptStatus;
	CColorStatic m_stTotalFileCountText;
	CColorStatic m_stFileProccessedCount;
	bool		m_bIsPopUpDisplayed;

	afx_msg void OnBnClickedCheckKeepOriginal();
};
