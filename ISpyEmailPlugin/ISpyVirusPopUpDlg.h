/**********************************************************************************************************      		Program Name          : ISpyVirusPopUpDlg.cpp
	Description           : This class is derived from CDialog which used to show when any virus got detected
							on machine it will prompt to user.
	Author Name			  : Neha Gharge
	Date Of Creation      : 07th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "resource.h"

// CISpyVirusPopUpDlg dialog

class CISpyVirusPopUpDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyVirusPopUpDlg)

public:
	CISpyVirusPopUpDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyVirusPopUpDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VIRUSSCAN_POPUP};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonMinimize();
	afx_msg void OnBnClickedButtonClose();
public:
	CColorStatic			m_stThreatName;
	CColorStatic			m_stScannedObject;
	CColorStatic			m_stSendersAddr;
	CColorStatic			m_stActionReq;
	CColorStatic			m_stThreatFound;
	CColorStatic			m_stObject;
	CColorStatic			m_stAddress;
	CComboBox				m_comboActionList;
	CStatic					m_stVirusScanHPic;
	CBitmap					m_bmpVirusScanPic;
	CxSkinButton			m_btnMinimize;
	CxSkinButton			m_btnClose;
	CxSkinButton			m_btnOk;
	CxSkinButton			m_btnCancel;

	CString					m_csThreatName;
	CString					m_csAttachmentName;
	CString					m_csSendersAddress;
	DWORD					m_dwAction;
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
