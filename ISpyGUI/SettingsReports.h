#pragma once
#include "afxwin.h"
#include "JpegDialog.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"


// CSettingsReports dialog

class CSettingsReports : public CJpegDialog
{
	DECLARE_DYNAMIC(CSettingsReports)

public:
	CSettingsReports(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsReports();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS_REPORTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CColorStatic m_stSelectDays;
	CComboBox m_CBDaysList;
	CxSkinButton m_btnOk;
	CxSkinButton m_btnCancel;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CxSkinButton m_btnClose;
	afx_msg void OnBnClickedButtonClose();
	BOOL WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey,CString strKey, DWORD dwChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait);
	DWORD ReadRegistryEntryofSetting();
};
