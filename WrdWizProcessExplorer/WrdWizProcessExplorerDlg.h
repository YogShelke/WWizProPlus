
// WrdWizProcessExplorerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <vector>
#include <Shlwapi.h>
#include <Imagehlp.h>
#include "JpegDialog.h"
#include "sciter-x-threads.h"
#include "WardWizDatabaseInterface.h"
#include "VERsionInfo.h"


/***************************************************************************************************
*  Class Name	  : CSciterBase
*  Description    : Sciterbase class for sciter DOM event handling
*  Author Name    : Sanjay
*  SR_NO		  :
*  Date			  : 12 Apr 2016
****************************************************************************************************/
class CSciterBase :
	public sciter::event_handler           // Sciter DOM event handling
{

};

// CWrdWizProcessExplorerDlg dialog
class CWrdWizProcessExplorerDlg : public CDialogEx
	, public CSciterBase,
	public sciter::host<CWrdWizProcessExplorerDlg> // Sciter host window primitives
{
// Construction
public:
	CWrdWizProcessExplorerDlg(CWnd* pParent = NULL);	// standard constructor
	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();

// Dialog Data
	enum { IDD = IDD_WRDWIZPROCESSEXPLORER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonMinimize();
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonClose();

	sciter::dom::element	m_root_el;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnCloseButton", On_OnCloseButton); // on clsoe button and on close image cross (X)
		FUNCTION_0("OnMinimize", On_Minimize) // On_Minimize()	
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_1("GetActiveProcessesName", GetActiveProcessesName)
		FUNCTION_1("GetStartUpProgram", GetStartUpProgram)
		FUNCTION_1("OnTerminateProcess", On_Terminate)
		FUNCTION_1("OnRemoveStartUp", On_RemoveStartUp)
		FUNCTION_2("GetCurrFileVersion", On_GetCurrFileVersion)
		FUNCTION_3("onModalLoop", onModalLoop)
	END_FUNCTION_MAP

	json::value On_OnCloseButton();
	json::value On_Minimize();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnGetAppPath();

	json::value GetActiveProcessesName(SCITER_VALUE svGetProcessesName);
	json::value GetStartUpProgram(SCITER_VALUE svGetStartUpProgram);
	json::value On_Terminate(SCITER_VALUE svProcess);
	json::value On_RemoveStartUp(SCITER_VALUE svProcess);
	json::value On_GetCurrFileVersion(SCITER_VALUE svPath, SCITER_VALUE svGetFileVersion);
	json::value onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal);

	SCITER_VALUE		m_svGetProcessesName;
	SCITER_VALUE		m_svGetStartUpProgram;
	SCITER_VALUE		m_svGetFileVersion;

};
