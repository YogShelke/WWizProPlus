#pragma once
#include "afxwin.h"
#include "JpegDialog.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"


// CSettingsScanTypeDlg dialog

class CSettingsScanTypeDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSettingsScanTypeDlg)

public:
	CSettingsScanTypeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsScanTypeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS_SCAN_TYPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CxSkinButton m_btnClose;
	CxSkinButton m_btnOk;
	CxSkinButton m_btnCancel;
	CComboBox m_CBScanType;
	CString m_ScanTypeS;
	CString m_ScanTypeF ;
	CColorStatic m_stSelectScanType;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	CString GetWardWizPathFromRegistry();
	//In Setting->Scan->Scan computer at start up->if i select full scan option then it shows Quick scan ,It should be full scan. 
	//Niranjan Deshak - 11/03/2015.
	DWORD ReadRegistryEntryofSetting();
	// reslove by lalit 1-14-2015 ,issue- handling of startup-scan from service, to resolve the issue of restart now popup coming frequently even after restart in xp
	bool SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, DWORD dwData, bool bWait);
};
