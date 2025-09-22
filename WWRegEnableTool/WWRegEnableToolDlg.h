/*********************************************************************
*  Program Name		: WWRegEnableToolDlg.h
*  Description		: CWWRegEnableToolDlg Implementation.
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 21/11/2019
**********************************************************************/

#include <atlbase.h>

#pragma once
using namespace std;

/***************************************************************************************************
*  Class Name	  : CSciterBase
*  Description    : Sciterbase class for sciter DOM event handling
*  Author Name    :	NITIN SHELAR
*  Date			  : 21/11/2019
****************************************************************************************************/
class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

/***************************************************************************************************
*  Class Name	  : CWWRegEnableToolDlg
*  Description    : CWWRegEnableToolDlg class for sciter DOM event, function map and method declaration.
*  Author Name    :	NITIN SHELAR
*  Date			  : 21/11/2019
****************************************************************************************************/
class CWWRegEnableToolDlg : public CDialogEx,
	public CSciterBase,
	public sciter::host <CWWRegEnableToolDlg>
{
// Construction
public:
	CWWRegEnableToolDlg(CWnd* pParent = NULL);	// standard constructor

	HWINDOW			get_hwnd();
	HINSTANCE		get_resource_instance();

	static CWWRegEnableToolDlg	*m_pThis;
// Dialog Data
	enum { IDD = IDD_WWREGENABLETOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedOk();
	afx_msg HCURSOR OnQueryDragIcon();
	
public:
	DECLARE_MESSAGE_MAP()

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetAppPath",OnGetAppPath)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("OnClose", On_Close)
		FUNCTION_0("OnMinimize", On_Minimize)
		FUNCTION_2("OnGetRegValue", On_GetRegValue)
		FUNCTION_2("OnChangeKeyValue", On_ChangeKeyValue)
		FUNCTION_0("OnGetRestart", On_GetRestart)
		FUNCTION_3("onModalLoop", onModalLoop)
	END_FUNCTION_MAP

	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnGetAppPath();
	json::value On_GetThemeID();
	json::value On_Close();
	json::value On_Minimize();
	json::value On_GetRestart();
	json::value On_GetRegValue(SCITER_VALUE scFunGetCurrentUserValueCB, SCITER_VALUE funSetChkVal);
	json::value On_ChangeKeyValue(SCITER_VALUE scfunSetMsgCB, SCITER_VALUE scFunSetCurrentUserValueCB);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);

public:

	HELEMENT				self;
	sciter::dom::element	m_root_el;
	CStringArray			csRegArray;
	bool					baRecArr[7];

	void			ReadCurrentUserRegValues(SCITER_VALUE scFunSetCurrentUserValueCB, SCITER_VALUE funSetChkVal);
	DWORD			SetRegistryValueData(HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD dwData);
	DWORD			ReadDwordRegVal(HKEY hKeyRoot, CString strKey, CString SubKey);

};
