#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "Resource.h"
#include "afxcmn.h"
#include "iTinEmailContants.h"
#include "ISpyDataManager.h"
#include "GradientStatic.h"
#include "FolderLockPassword.h"


// CISpyFolderLocker dialog

class CISpyFolderLocker : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyFolderLocker)

public:
	CISpyFolderLocker(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyFolderLocker();

// Dialog Data
	enum { IDD = IDD_DIALOG_FOLDERLOCKER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	HMODULE		m_hRegisterDLL ;
public:
	CFolderLockPassword m_objFolderLockPassword;
	CString m_csPathName;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	LRESULT OnNcHitTest(CPoint point);
	CStatic m_stHeaderFolderLockPic;
	HBITMAP	m_bmpHeaderFolderLockPic;
	CxSkinButton m_btnBack;
	HCURSOR	m_hButtonCursor;
	afx_msg void OnBnClickedButtonFolderlockBack();
	CEdit m_edtFolderLockPath;
	CxSkinButton m_btnFolderLockBrowse;
	afx_msg void OnBnClickedButtonFolderBrowse();
	CComboBox m_comboFolderLockOption;
	CColorStatic m_stInfoText;
	CListCtrl m_lstFolderLock;
	CxSkinButton m_btnFolderOpen;
	CxSkinButton m_btnFolderRemove;
	//CFont*			m_pTextFont;
	CDataManager    m_objFolderLocker;
	CxSkinButton m_btnStartFolderAction;
	afx_msg void OnBnClickedButtonFolderopen();
	afx_msg void OnBnClickedButtonFolderremove();
	afx_msg void OnCbnSelendokComboOptionFolderlock();
	afx_msg void OnBnClickedButtonStartFolderaction();
	void InsertItem(CString csFilePath);
	bool SendFolderEntries2Service(DWORD dwAddEditDelType, DWORD dwType, CString csEntry, bool bFolderLockWait= false);
	void LoadExistingFolderLockerEntriesFile();
	void LoadDataContentFromFile(CString csPathName);
	void PopulateList();
	DWORD Str2Hex( CString const & s );
	afx_msg void OnLvnItemchangedListFolderlock(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckBox();
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName);
	bool ValidateGivenPassWord(CString csPassword);
	DWORD ValidatePassword(CString csPassword);
	void GetEnviormentVariablesForAllMachine();
	bool CheckingWithEnviormentPath(CString csPathName);
	void ResetControlOfFolder();
	bool AvoidOSPath();
	void LoadRegistartionDll();
	int PopUpDialog();
	bool isAnyEntrySeletected();
	bool CheckIsParentFolderLocked(wchar_t *szFullPath);
	CButton m_chkSelectAll;
	CColorStatic m_stSelectAll;
	DWORD	m_dwLockpasswrd;
	bool    m_bUnlockingStop;
	bool    m_bUnlockingThreadStart;
	TCHAR	m_szProgramData[512] ;
	//TCHAR	m_szUserProfile[512] ;
	TCHAR	m_szWindow[512] ;
	TCHAR   m_szProgramFile[512];
	TCHAR   m_szProgramFilex86[512];
	//TCHAR	m_szAppData[512] ;
	TCHAR	m_szAppData4[512] ;
	TCHAR	m_szDriveName[512] ;
	TCHAR	m_szArchitecture[512] ;
	TCHAR	m_szBrowseFilePath[MAX_PATH];
	CColorStatic m_stFolderLlckrSubHeader;
	CColorStatic m_stFolderLlckrHeader;
	bool	m_bIsPopUpDisplayed;
	void RefreshStrings();
	bool OnBnClickFolderLock();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
};
