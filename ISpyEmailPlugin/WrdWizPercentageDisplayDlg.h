#pragma once

#include "afxwin.h"
#include "afxcmn.h"
#include "JpegDialog.h"
#include "ColorStatic.h"
#include "PictureEx.h"
#include "xSkinButton.h"
#include "resource.h"

// CWrdWizPercentageDisplayDlg dialog
#define TIMER_MOVEMAIL_STATUS 340

class CWrdWizPercentageDisplayDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CWrdWizPercentageDisplayDlg)

public:
	CWrdWizPercentageDisplayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWrdWizPercentageDisplayDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PERCETAGE_SHOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
public:
	bool	m_bCancelProcess;
	bool	m_bInitializeDlg;
	CColorStatic m_stPleaseWaitMsg;
	CProgressCtrl m_PrgToShowMovingMailProcess;
	DWORD m_dwTotalMailCount;
	DWORD m_dwCurrentMoveMailCount;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void EndDialogOFDisplay();
	
	
	CxSkinButton m_btnCancel;
	CPictureEx m_GifMovingMail;
	afx_msg void OnBnClickedButtonCancel();
	DECLARE_MESSAGE_MAP()
};
