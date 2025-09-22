#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

// CPopUpWithReminder dialog

class CPopUpRenewProduct : public CDialog
	, public CSciterBase,
	public sciter::host<CPopUpRenewProduct>
{
	DECLARE_DYNAMIC(CPopUpRenewProduct)

public:
	CPopUpRenewProduct(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPopUpRenewProduct();

	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// Dialog Data
	enum { IDD = IDD_DIALOG_RENEW_PRODUCT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	LPTSTR GetRegisteredEmailID();
	LPTSTR GetRegisteredUserInformation();
	DWORD GetRegistrationDataFromRegistry();
	DWORD GetRegistrationDatafromFile();
	CString GetModuleFilePath();
	DWORD GetRegistrationDatafromFile(CString strUserRegFile);
	DWORD DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize);


	TCHAR						m_szRegKey[32];
	int							m_iDisabled;
	DWORD 						m_dwNoofDays;


	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	
	sciter::dom::element m_root_el;

	SCITER_VALUE m_svProductDetails;

	CString			 m_szEmailId_GUI;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("onRenewProduct", OnRenewProduct);
		FUNCTION_1("onLoadProductDetails", OnLoadProductDetails);
		FUNCTION_0("GetProductID", On_GetProductID);
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID);
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value OnRenewProduct();
	json::value OnLoadProductDetails(SCITER_VALUE svProductDetails);
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
};
