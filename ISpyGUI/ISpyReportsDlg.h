/****************************************************
*  Program Name: ISpyReportsDlg.h                                                                                                   
*  Author Name: Prajakta
*  Date Of Creation: 20 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "afxcmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <io.h>

// CISpyReportsDlg dialog


/****************************************************
CLASS DECLARATION,MEMBER VARIABLE & FUNCTION DECLARATION
****************************************************/

class CISpyReportsDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyReportsDlg)

public:
	CISpyReportsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyReportsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REPORTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	bool LoadDBFile();
	void PopulateList();
	void ParseCurrentLine(CString csLine);
	void InsertCurrentString(int iItemNo, CString csItemString);
	void ShowControls4Reports();
	void ShowHideControls(bool bEnable);
	void AddEntriesInReportsDBAfterDelete(CString csDateTime, CString csScanType, CString csThreatName, CString csFilePath, CString csAction);
	bool SaveInReportsDB();
	bool SendReportsOperation2Service(DWORD dwType, CString csDateTime, CString csScanType, CString csFilePath, bool bReportsWait = false);
	bool SendReportsData2Service(DWORD dwMessageInfo, DWORD dwType, CString csEntry, bool bReportsWait = false);
	bool StopReportOperation();
	afx_msg void OnBnClickedCheckSelectall();
	afx_msg void OnBnClickedBtnBack();
	afx_msg void OnBnClickedBtnDelete();
	void RefreshStrings();
	void OnBnClickedReports();
	bool GetSortedFileNames(vector<int> &vec);
	bool GetReportsFolderPath(CString &csFolderPath);
	void GetFileDigits(LPCTSTR pstr, vector<int> &vec);
	void LoadRemainingEntries();
	bool SaveDBFile();
	bool LoadDataContentFromFile(CString csPathName);
	bool GetDateTimeFromString(CString csItemEntry, CTime &objDateTime);

	CStatic			m_stHReports;
	HBITMAP			m_bmpReports;
	CButton			m_btnChkbox;
	CButton			m_chkSelectAll;
	CStatic			m_stMsg;
	CxSkinButton	m_btnBack;
	CxSkinButton	m_btnDelete;
	CListCtrl		m_lstListView;
	HCURSOR			m_hButtonCursor;
	bool			m_bSuccess;
	bool			m_bReportHome;
	bool			m_bReportClose;
	bool			m_bReportStop;
	bool			m_bReportThreadStart;
	CDataManager	m_objReportsDB;
	CDataManager	m_objReportsDBToSave;
	
	CColorStatic	m_stSelectAll;
	CColorStatic	m_stReportsHeaderName;
	HANDLE			m_hReportsThread;
	CColorStatic	m_stHeaderReportDes;
	bool			m_bDeleteEntriesFinished;
	bool			m_bIsPopUpDisplayed;
	CISpyCommunicator m_objCom;

	afx_msg void OnPaint();
	// issue number: 19, resolved by lalit kumawat, issue: In report form while pressing the enter ,the report contents will not working.

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//Issue No.16 While report deletion is in progress if I click Report button then Report does not takes place
	// Gaurav Chopdar Date:2/1/2015
	bool m_bReportEnableDisable;
};
