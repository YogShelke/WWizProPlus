#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "CpuUsage.h"

// CWardWizWidgetDlg dialog

class CWardWizWidgetDlg : public CDialog
	, public CSciterBase,
	public sciter::host<CWardWizWidgetDlg>
{
	DECLARE_DYNAMIC(CWardWizWidgetDlg)

public:
	CWardWizWidgetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizWidgetDlg();

	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Dialog Data
	enum { IDD = IDD_WARDWIZWIDGETDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	sciter::dom::element m_root_el;
	CString				m_lpstrDate;
	CString				m_csRegKeyPath;
	CString				m_csUpdateDate;
	DWORD				m_dwTotalFiles;
	TCHAR				m_szSpecificPath[512];
	DWORD				dwStartBytes;
	DWORD				dwTotalFileSize;
	double				m_dTotalFileSize;
	HANDLE				m_hWaitEvent;

	BEGIN_FUNCTION_MAP
		//FUNCTION_0("Getpath", Getpath)
		FUNCTION_0("OnClickUpdate", On_ClickUpdate)
		FUNCTION_0("OnClickTempFileCleaner", On_ClickTempFileCleaner)
		FUNCTION_0("OnBnClickedButtonScanPage", On_BnClickedButtonScanPage)
		FUNCTION_0("OnClickReport", On_ClickReport)
		FUNCTION_1("funUnRegProd", On_funUnRegProd)
		FUNCTION_2("funActiveScan", On_funActiveScan)
		FUNCTION_1("funUptoDateProd", On_funUptoDateProd)
		FUNCTION_0("GetLanguageID", GetLanguageID)
		FUNCTION_1("GetCpuUsage", GetCpuUsage)
		FUNCTION_0("On_GetProductID", On_GetProductID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("HideWidget", On_HideWidget)
		FUNCTION_0("DisableWidget", On_DisableWidget)

	END_FUNCTION_MAP

	//json::value Getpath();
	json::value On_ClickUpdate();
	json::value On_ClickTempFileCleaner();
	json::value On_BnClickedButtonScanPage();
	json::value On_ClickReport();
	json::value On_funUnRegProd(SCITER_VALUE svDays);
	json::value On_funActiveScan(SCITER_VALUE svSetWidgetACState, SCITER_VALUE svGetTempFoldSize);
	json::value On_funUptoDateProd(SCITER_VALUE svGetProdUpdtInfo);
	json::value GetLanguageID();
	json::value GetCpuUsage(SCITER_VALUE svGetCpuUsageCount);
	json::value On_GetProductID();
	json::value On_GetThemeID();
	json::value On_HideWidget();
	json::value On_DisableWidget();

	SCITER_VALUE m_svSetWidgetACState;
	SCITER_VALUE m_svGetTempFoldSize;
	SCITER_VALUE m_svDays;
	SCITER_VALUE m_svWidgetShowHide;
	SCITER_VALUE m_svGetProdUpdtInfo;
	SCITER_VALUE m_svGetCpuUsageCount;
	CCpuUsage	m_cpuUSage;
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonDownloadNow();
	bool SendData2UI(int iMessageInfo, bool bWait = false);
	bool SendData2UI(int iMessageInfo, int iParamFirst, bool bWait = false);
	void OnBnClickedButtonScanNow();
	void OnBnClickedButtonReports();
	void WidgetActiveScanColorState(DWORD dwFlagState);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait = true);
	bool SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam = NULL, LPTSTR lpszSecondParam = NULL, bool bWait = false);
	void GetDaysRegLeft(int iRetDays);
	void GetDaysLeftOnHomeBtn();
	void EnumFolder(LPCTSTR lpFolPath);
	void InitilizeZero();
	void CallGetTotalTempFolderSize();
	void CallGetProdUpdtInfo();
	void CloseUI();
	bool isValidDate(int iDay, int iMonth, int iYear);
	void Show_CPU_Usage();
	void StartRequiredThreads();
	bool CheckIsAppRunning(LPTSTR);
};
