/**********************************************************************************************************                     
	  Program Name          : WWizUninstThirdDlg.h
	  Description           : Third Page of Uninstaller.
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 6th Feb 2015
	  Version No            : 1.9.0.0
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "Enumprocess.h"
#include "ColorStatic.h"
#include "xSkinButton.h"


class CWWizUninstThirdDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CWWizUninstThirdDlg)

public:
	CWWizUninstThirdDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWWizUninstThirdDlg();

// Dialog Data
	enum { IDD = IDD_UNINSTALL_THIRDPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_stUninstallCompleted;
	CStatic m_bmpFinishBMP;
	CxSkinButton m_btnRestartNow;
	CFont m_BoldText;
	afx_msg void OnBnClickedButtonRestartNow();
	afx_msg void OnBnClickedButtonRestartLater();
	virtual BOOL OnInitDialog();
	CColorStatic m_stSuccessMsg;
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnBackground(CBitmap* bmpImg, CDC* pDC,int yHight,bool isStretch);
	HBITMAP			m_bmpCompletedImg;
	CColorStatic m_stSuccessMsg2;
	CStatic m_stUninstallConfirmTitle;
	CStatic m_stAllRightReserved;
	CxSkinButton m_btRestartLetr;
};
