/**********************************************************************************************************
Program Name          : PatchMerger.h
Description           : 
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 22 Mar 2016
Version No            : 1.13.1.10
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CPatchMerger dialog

class CPatchMerger : public CDialog
{
	DECLARE_DYNAMIC(CPatchMerger)

public:
	CPatchMerger(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPatchMerger();

// Dialog Data
	enum { IDD = IDD_DIALOG_PATCHMERGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonCompare();
	afx_msg void OnBnClickedCheckBselectall();
	afx_msg void OnBnClickedCheckEselectall();
	afx_msg void OnBnClickedCheckPselectall();
	afx_msg void OnBnClickedCheckEliteClientselectall();
	afx_msg void OnBnClickedCheckEliteServerselectall();
	afx_msg void OnBnClickedCheckEssentialPlusselectall();
	afx_msg void OnBnClickedButtonLbrowse();
	afx_msg void OnBnClickedButtonRbrowse();
	afx_msg void OnEnChangeEditLeftFolder();
	afx_msg void OnEnChangeEditRightFolder();
	afx_msg void OnBnClickedButtonMerge();
public:
	virtual BOOL OnInitDialog();
	void SetStatus(CString csStatus);
	void InitializeVariables();
	CString GetSelectedFolder();
	bool CompareFolder(CString csLPath, CString csRPath);
	bool CompareFiles(DWORD dwProdID, CString csLPath, CString csRPath);
	bool CompareSections(DWORD dwProductID, CString csSectionName, CString csLPath, CString csRPath);
	bool ParseIniLine(LPTSTR lpszIniLine, LPTSTR lpExeName, LPTSTR lpszZipSize, LPTSTR lpszFullSize, LPTSTR lpszFileHash);
	bool ISDifferenceWithSecondFile(CString csFilePath, LPTSTR lpszIniLine, LPTSTR lpExeName, LPTSTR lpZipSize, LPTSTR lpFullSize, LPTSTR lpFileHash, DWORD &dwIndex);
	bool InsertInList(DWORD dwProductID, LPTSTR szSectionName, LPTSTR szData, DWORD &dwIndex);
	void SelectAll(CListCtrl &objlst, BOOL bSelect);
	void CheckValidPaths();
	bool MergeContents(CString csFilePath, DWORD dwProdID, CListCtrl &objlst);
	bool WriteEntryInDestinationFile(CString csFilePath, CString csSection, CString csKey, CString csData);
	bool IsItemsChecked(CListCtrl &objlst);
	bool GetNewKeyFromDestinationFile(CString csFilePath, CString csSection, CString &csKey);
	bool CopyFile2Destination(CString csData, CString csSection, DWORD dwProdID);
	bool ISDifferenceWithSecondFile(CString csFilePath, LPTSTR lpszIniLine, LPTSTR lpszKey, LPTSTR lpszData);
public:
	CEdit			m_editLeftFolder;
	CEdit			m_editRightFolder;
	CButton			m_btnLeftBrowse;
	CButton			m_btnRightBrowse;
	CTabCtrl		m_tabProdTypes;
	CButton			m_btnCompare;
	CButton			m_btnMerge;
	CEdit			m_editStatus;
	CListCtrl		m_lstBasic;
	CListCtrl		m_lstEssential;
	CListCtrl		m_lstPro;
	CButton			m_chkBasic;
	CButton			m_chkEssential;
	CButton			m_chkPro;
	CStatic			m_stBasicCount;
	CStatic			m_stEssentialCount;
	CStatic			m_stProCount;
	CStatic         m_stEliteCount;
	CStatic         m_stEssPlusCount;
	CListCtrl m_lstElite;
	CListCtrl m_lstEssPlus;
	CListCtrl m_lstEliteServer;
	CStatic m_stEliteServer;
//	CButton m_chkElite;
//	CButton m_chkEliteServer;
//	BOOL m_chkEssPlus;
//	int m_chkEssPlus;
//	CButton m_chkElite;
	CButton m_chkEliteClient;
	CButton m_chkEliteServer;
	CButton m_chkEssentialPlus;
};
