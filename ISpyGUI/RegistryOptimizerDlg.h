/****************************************************
*  Program Name: RegistryOptimizerDlg.h                                                                                                   
*  Author Name: Vilas & Prajakta                                                                                                      
*  Date Of Creation: 16 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/
#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "ColorStatic.h"
#include "xSkinButton.h"
#include "RegOptStruct.h"
#include "TextProgressCtrl.h"
#include "iSpyMemMapClient.h"
#include "GradientStatic.h"


// CRegistryOptimizerDlg dialog



/*
typedef struct _REGOPTSCANOPTIONS
{
	
	bool	bActiveX ;					//Check Invalid ActiveX Entries. 0->Not, 1->Scan & repair
	bool	bUninstall ;
	bool	bFont ;
	bool	bSharedLibraries ;
	bool	bApplicationPaths ;
	bool	bHelpFiles ;
	bool	bStartup ;
	bool	bServices ;
	bool	bExtensions ;
	bool	bRootKit ;
	bool	bRogueApplications ;
	bool	bWorm ;
	bool	bSpywares ;
	bool	bAdwares ;
	bool	bKeyLogger ;
	bool	bBHO ;
	bool	bExplorer ;
	bool	bIExplorer ;

	DWORD	dwStats[0x12] ;			

}REGOPTSCANOPTIONS, *LPREGOPTSCANOPTIONS ;
*/

/****************************************************
CLASS DECLARATION,MEMBER VARIABLE & FUNCTION DECLARATION
****************************************************/


class CRegistryOptimizerDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CRegistryOptimizerDlg)

public:
	CRegistryOptimizerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegistryOptimizerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REGISTRYOPTIMIZER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	CColorStatic	m_stRepairedEntries;
	CColorStatic    m_stRegScanArea;
	CxSkinButton	m_btnStop;
	CxSkinButton	m_btnScanRepair;
	CxSkinButton    m_btnBack;
	CColorStatic	m_stActiveX;
	CColorStatic	m_stUninstall;
	CColorStatic	m_stFontEnt;
	CColorStatic	m_stSharedDll;
	CColorStatic	m_stAppPath;
	CColorStatic	m_stHelpFileInfo;
	CColorStatic	m_stStartup;
	CColorStatic	m_stWinServices;
	CColorStatic	m_stInvalidExt;
	CColorStatic	m_stRootkits;
	CColorStatic	m_stRogueApp;
	CColorStatic	m_stWorms;
	CColorStatic	m_stSpywareThreats;
	CColorStatic	m_stAdwareThreats;
	CColorStatic	m_stKeyloggers;
	CColorStatic	m_stBHO;
	CColorStatic	m_stExplorerEnt;
	CColorStatic	m_stInternetExpEnt;
	CColorStatic	m_Static_Percentage;
	HCURSOR			m_hButtonCursor;
	bool			m_bScanCompleted;
	DWORD			m_dwPercentage;
	DWORD			m_dwTotalEntries;
	HANDLE			m_hRegOptThread;

	CButton m_btnActiveX;
	CButton m_btnUninstall;
	CButton m_btnFontEnt;
	CButton m_btnSharedDll;
	CButton m_btnAppPath;
	CButton m_btnHelpFileInfo;
	CButton m_btnWinStartup;
	CButton m_btnWinServices;
	CButton m_btnInvalidExt;
	CButton m_btnRootkits;
	CButton m_btnRogueApp;
	CButton m_btnWorms;
	CButton m_btnSpyThreats;
	CButton m_btnAdwareThreats;
	CButton m_btnKeyloggers;
	CButton m_btnBHO;
	CButton m_btnExplorerEnt;
	CButton m_btnInternetExpEnt;
	CTextProgressCtrl	m_prgStatus;
	CStatic m_stHRegOpt;
	HBITMAP m_bmpRegOpt;
	HMODULE m_hRegOptDLL;
	bool	m_bScanStarted;
	bool	m_bScanStop;
	bool    m_bClose;
	bool	m_bRegOptHome;
	DWORD	m_dwTotalRepairedEntries;
	bool	m_bIsPopUpDisplayed;

	iSpyServerMemMap_Client m_objiTinServerMemMap_Client;
	CColorStatic m_stRegOptHeaderDesc;

	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnNMCustomdrawProgressStatus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnScanrepair();
	afx_msg void OnStnClickedStaticEntries();
	afx_msg void OnStnClickedStHregopt();
	afx_msg void OnStnClickedStaticRegscanarea();
	afx_msg void OnBnClickedBtnBack();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	bool ShutDownScanning() ;
	void ResetControls();
	void ResetRegistryScanOption();
	bool GetRegOptScanAreaOptions( LPREGOPTSCANOPTIONS ) ;
	void GetDWORDFromScanOptions( DWORD &dwRegScanOpt, LPREGOPTSCANOPTIONS lpRegOpt);
	void ScanningStopped();
	bool GetPercentageNTotalEntries(LPTSTR szEntry, DWORD &dwPercentage, DWORD &dwTotalEntries);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CColorStatic m_stRegistryHeaderName;
	void RefreshStrings();
	CButton m_chkSelectAll;
	CColorStatic m_stSelectAll;
	afx_msg void OnBnClickedCheckSelectall();
	CGradientStatic m_stTotalRepairedEntries;
	afx_msg void OnPaint();
	bool PauseRegistryOptimizer();
	bool ResumeRegistryOptimizer();
};
