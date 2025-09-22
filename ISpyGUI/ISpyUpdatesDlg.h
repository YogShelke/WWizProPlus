/*******************************************************************************************
*  Program Name: ISpyUpdateDlg.h                                                                                                  
*  
*  Author Name:		1)Ramkrushna
*					2)Neha Gharge
*  Date Of Creation: 15-0ct 2013                                                                                              
*  Version No:		 1.0.0.2   
/*********************************************************************/ 
//HEADER FILE
/**********************************************************************/

#pragma once
#include "ISpyLiveUpSecondDlg.h"
#include "DownloadConts.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "ColorStatic.h"
#include "xSkinButton.h"
#include "GradientStatic.h"


/*********************************************************************
CLASS DECLARATION,FUCTION DECLARATION,MEMEBER VARIABLE DECLARATION

**********************************************************************/
class CISpyUpdatesDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyUpdatesDlg)

public:
	CISpyUpdatesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyUpdatesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_UPDATES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedRadioFrominternet();
	afx_msg void OnBnClickedRadioFromlocalfolder();
	afx_msg void OnBnClickedButtonBrowsebutton();
	void EnableControls(BOOL bEnable);
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonLiveupdatehelp();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedBackButton();
	void ShowHideAllUpdateFirstPageControls(bool bEnable);
	afx_msg void OnBnClickedUpadteCancel();
	afx_msg void OnBnClickedButtonHelp();
	bool CheckForValidUpdatedFiles(CString csInputFolder, std::vector<CString> &csVectInputFiles);
	afx_msg void OnBnClickedUpdateBrowse();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void ShowHideToolsDlgControls(bool bEnable);
	void ShowOnlyFirstWindow();
	void ResetControls();
	void ShowPageToDisplay();
	//Modified: 
	//Issue No 107	Doesn’t check the contents of the DB file. Even if DB contains garbage its accepted
	//ITin_Developement_Release_1.0.0.3_Patches
	//Developer: Nitin Kolapkar
	DWORD ValidateDB_File( TCHAR *m_szFilePath, DWORD &dwStatus, CString &csInputPathProgramData) ;
	DWORD CheckForValidVersion(CString csVersionNo);
	DWORD ReadDBVersionFromReg();
	bool UpdateVersionIntoRegistry();
	DWORD CheckForFileSize(CString csFilename, DWORD dwDBVersionLength);
	bool ValidateFileNVersion(CString csFileName, CString csVersion, DWORD dwDBVersionLength);
	void EnumFolder(LPCTSTR pstr);
	bool IsPopUpDisplayed();
public:
	CString					m_csFileName;
	DWORD					m_dwCurrentLocalFileSize;
	CString					m_CurrentFilePath;
	int						m_iTotalNoofFiles;
	CStringArray			m_csFilesList;
	CString					m_csVersionNo;
	//CString					m_csWholesignature;
	CEdit					m_editFolderPath;
	CxSkinButton			m_btnFromInternet;
	CxSkinButton			m_btnFromLocalComputer;
	eLIVE_UPDATE_TYPE		m_updateType;
	CString					m_csInputFolderPath;
	CISpyLiveUpSecondDlg	m_dlgISpyUpdatesSecond;
	CxSkinButton			m_btnBack;
	CxSkinButton			m_btnNext;
	CxSkinButton			m_btnHelp;
	CxSkinButton			m_btnCancel;
	CxSkinButton			m_btnBrowse;
	CStatic 				m_HeaderPic;
	HBITMAP 				m_bmpHeaderPicture;
	CColorStatic 			m_stBtmText;
	CColorStatic 			m_stTopText;
	int 					m_RLocalFolder;
	int 					m_RUpdateInternet;
	HCURSOR					m_hButtonCursor;
	CColorStatic			m_stInternetText;
	CColorStatic			m_stLocalFolder;
	int						m_iUpdateFailed;
	int						m_iSignatureFailed;
	bool					m_bOlderdatabase;
	bool					m_bdatabaseuptodate;
	bool					m_bIsPopUpDisplayed;

	DWORD					m_dwMaxVersionInZip;
	iSpyServerMemMap_Client m_objIPCALUpdateClient;
	CISpyCommunicator m_objCom;
	void RefreshStrings();
	CColorStatic m_stUpdatesHeader;
	afx_msg void OnPaint();
	bool StartALUpdateUsingALupdateService();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CString ExtractRARFile(CString csInputFileName, DWORD &dwUnzipCount);
	bool EnumAndDeleteTempFolder(CString csInputFileName);
	bool CheckForMaxVersionInZip(CString csVersionNo);
	CString GetAppFolderPath();
	bool CreateDirectoryFocefully(LPTSTR lpszPath);
};
