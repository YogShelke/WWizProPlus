#pragma once
//#include "stdafx.h"
#include <afxtempl.h>
#include "xSkinButton.h"
#include "flattooltipctrl.h"
#include "ScanDlg.h"
#include "ISpyRecoverDlg.h"
#include "ISpyReportsDlg.h"
#include "ISpyEmailScanDlg.h"
#include "RegistryOptimizerDlg.h"
#include "ISpyAntiRootkit.h"
#include "ISpyFolderLocker.h"
#include "RegistryOptimizerDlg.h"
#include "ISpyUpdatesDlg.h"
#include "DataEncryptionDlg.h"
#include "WardWizUtilitiesDlg.h"
enum
{
	QUICK_SCAN_DLG = 1,
	FULL_SCAN_DLG,
	CUSTOM_SCAN_DLG,
	ANTIROOTKIT_SCAN_DLG,
	REGISTRY_OPTIMIZER_DLG,
	DATA_ENCRYPTION_DLG,
	RECOVER_DLG,
	FOLDER_LOCKER_DLG,
	EMAIL_VIRUS_SCAN_DLG,
	EMAIL_SPAM_FILTER_DLG,
	EMAIL_CONTENT_FILTER_DLG,
	EMAIL_SIGNATURE_DLG,
	WARDWIZ_UPDATES_DLG,
	WARDWIZ_REPORTS_DLG ,
	WARDWIZ_UTILITY_DLG
};

enum
{
	SETTINGS_GENERAL_DLG = 0,
	SETTINGS_SCAN_DLG,
	SETTINGS_EMAIL_DLG,
	SETTINGS_UPDATE_DLG
};

class CTabDialog : public CDialog
{
public:
	CTabDialog(UINT nID, CWnd* pParent /* = NULL*/);   // standard constructor
	~CTabDialog();   //Destructor

	int m_iDlgCx;
	int m_iDlgCy;
	void DisableAllBtn();
	void EnableAllBtn();
	void DisableAllExceptSelected();

	//Resolved Issue No. 30 While Cleaning Threats, Other scan options should be disabled.
	//-Niranjan Deshak. 25/12/2014. 
	void EnableAllExceptSelected();

	
	DWORD m_SelectedButtonSetting;

	BOOL AddPage(CDialog* pDialog, CxSkinButton* pButton, int iStringId = -1);
	BOOL PreTranslateMessage(MSG* pMsg);
	void InitPagesShow();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void AttachToolTip(CxSkinButton &objButton, int iStringId);	
	DWORD GetSelectedDialog(int i);
	DWORD GetSelectedDialogSetting(int i);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();

private:
	CToolTipCtrlEx *m_pToolTip;
	int m_nTotalPage;
	int m_iPreSelected;
	
	DWORD m_dwPrevSelectedButton;
public:
	int m_iCurrentSelectedButton;
	DWORD m_SelectedButton;

private:
	int	m_iCurrentSelectedButtonSetting; 
	CMap<int, int&, CDialog*, CDialog*> m_DialogMap;
	CMap<int, int&, CxSkinButton*, CxSkinButton*> m_ButtonMap;

	void InitButtonsShow();
	void InitDialogShow();
	void HideAllPages();
public:
	void SetSelectedSkin(int iButton);
	bool IsEmailScanDlgActive();

private:
	void SetSelectedSkinSetting(int iButton);
	void DisableUnselectedButton(int iButton);
	

public:
	void SetDialogType(DWORD dwDlgType);
	DWORD	m_dlgType;
	DWORD   m_dwEmailScanEnable;
	bool	m_bOutlookInstalled;
	bool    m_bUnRegisterProduct;
	void	SetSelectedButton(DWORD dwButton);
	DECLARE_MESSAGE_MAP()
};
