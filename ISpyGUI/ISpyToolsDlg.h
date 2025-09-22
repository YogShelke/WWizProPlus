/****************************************************
*  Program Name: ISpyToolsDlg.h                                                                                                   
*  Author Name: Neha Gharge                                                                                                     
*  Date Of Creation: 20 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/

#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "RegistryOptimizerDlg.h"
#include "DataEncryptionDlg.h"
#include "ISpyRecoverDlg.h"
#include "ISpyFolderLocker.h"



/****************************************************
CLASS DECLARATION,MEMBER VARIABLE & FUNCTION DECLARATION
****************************************************/

class CISpyToolsDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyToolsDlg)

public:
	CISpyToolsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyToolsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TOOLS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonRegistryopt();
	afx_msg void OnBnClickedButtonDataencrypt();
	afx_msg void OnBnClickedButtonToolback();		
	afx_msg void OnBnClickedButtonRecover();
	void ShowHideToolsDlgControls(bool bEnable);
	void HideChildWindows();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
public:
	CxSkinButton			m_btnRegistryOpt;
	CxSkinButton			m_btnDataEncrypt;
	CxSkinButton			m_btnToolBack;
	CxSkinButton			m_btnRecover;
	CxSkinButton			m_btnFolderLocker;
	CStatic					m_stToolHeader;
	CBitmap					m_bmpToolHeader;
	//CRegistryOptimizerDlg	m_objRegistryOptDlg;
	//CDataEncryptionDlg		m_objDataEncryptionDlg;
	//CISpyRecoverDlg			m_objRecoverDlg;
	//CISpyFolderLocker		m_objFolderLocker;
	HCURSOR					m_hButtonCursor;
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	afx_msg void OnBnClickedButtonFolderlock();
};
