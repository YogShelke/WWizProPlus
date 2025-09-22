/**********************************************************************************************************                     
	  Program Name          : WWizUninstSecondDlg.h
	  Description           : 
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 6th Feb 2015
	  Version No            : 1.9.0.0
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#pragma once
#include "JpegDialog.h"
#include "afxcmn.h"
#include "WWizUninstThirdDlg.h"
#include "afxwin.h"
#include "ColorStatic.h"
#include "xSkinButton.h"
#include "TextProgressCtrl.h"

class CWWizUninstSecondDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CWWizUninstSecondDlg)

public:
	CWWizUninstSecondDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWWizUninstSecondDlg();
void RemoveDriverfilesFromSystem32();
// Dialog Data
	enum { IDD = IDD_UNINSTALL_SECONDPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	HICON m_hIcon;
	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	
	bool DeleteFiles(std::vector<CString>	&vFilePathLists);
	bool DeleteFolder(std::vector<CString>	&vFolderPathLists);
	void EnumFolderToCountFiles(LPCTSTR pstr);// bool m_bQuarantine, bool m_bPassDbMngr);
	bool DeleteRegistryKeys();
	void StartUninstallProgram();
	void DeleteFinished();
	void UninstallationMessage();
	void ShowHideUninstSecondDlg(bool bEnable);
	bool GetCloseAction4OutlookIfRunning(bool bIsReInstall);
	bool CloseAllAplication(bool bIsReInstall);
	void OnBackground(CBitmap* bmpImg, CDC* pDC,int yHight,bool isStretch);
	void OnBackgroundMiddleImage(CBitmap* bmpImg, CDC* pDC, int yHight, bool isStretch);
	void StopAndUninstallProcessProtection(int OsType);
	void StopAndUninstallScanner();
	void StopProtectionDrivers();
	void RemoveDriverRegistryKeyAndPauseDrivers();
	BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey);
	BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey);
	void UnregisterComDll(CString csAppPath);

public:
	std::vector<CString>		m_vFilePathLists;
	std::vector<CString>		m_vFolderPathLists;
	int							m_iTotalFileCount;
	int							m_iTotalDeletedFileCount;
	bool						m_bRestartReq;
	bool						m_bMoveFileExFlag;
	CStringArray				m_csArrRegistryEntriesDel;

	HANDLE						m_hThreadUninstall;
	CString						m_csProductName;
	CString						m_csAppPath;
	CTextProgressCtrl				m_pcDeleteProgress;
	CWWizUninstThirdDlg			m_objWWizUninstThirdDlg;
	CStatic					    m_stConfigMessage;
	HBITMAP						m_bmpImageMinimize;
	CxSkinButton m_btnMinimizeToTaskBar;
	afx_msg void OnBnClickedButtonMinimizeToTaskbar();
	CStatic m_stUninstallationInProgress;
	CStatic m_stMinimizeWindowText1;
	CStatic m_stMinimizeWindowText2;
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	CxSkinButton m_closeBtn;
	afx_msg void OnBnClickedButtonClose();
	void DeleteAllAppShortcut();
	DWORD RemoveFilesUsingSHFileOperation(TCHAR *pFolder );
	CStatic						m_stImageMinimize;
};
