#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"

typedef struct __tagThreatInfor4EmailScan
{
	TCHAR  szThreatName[MAX_PATH];
	TCHAR  szFilePath[MAX_PATH];
	TCHAR  szSenderAddr[MAX_PATH];
	DWORD   dwActionTaken;
}THREATINFO4EmailScan;

typedef std::vector<THREATINFO4EmailScan> VECTHREATINFO4EmailScan;
class CEmailNotifyTrayDlg : public CDialog
	, public CSciterBase,
	public sciter::host<CEmailNotifyTrayDlg>
{
public:
	CEmailNotifyTrayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEmailNotifyTrayDlg();
	
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	enum { IDD = IDD_WWEMAILSCANTRAYDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	sciter::dom::element m_root_el;

	int GetTaskBarWidth();
	int GetTaskBarHeight();
	void GetEmailScanData(LPCTSTR szThreatName, LPCTSTR szAttachmentName, LPCTSTR szSenderAddr, DWORD dwAction);
	bool AddIntoQueueEmailScanInfo(CString csThreatName, CString scAttachmentName, CString scSenderAddr, DWORD dwActionTaken);
	void AddQuarantineInfo(CString, CString, CString, int, int);
	VECTHREATINFO4EmailScan		m_vecThreatList;
	int					m_iSize;
	bool				bIsNextPrevClicked;
	int					m_iCurrentIndex;
	bool				m_bIsCloseClicked;
	bool				m_bFirstEntry;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnBtnClose", OnBTNCilckClose)
		FUNCTION_0("GetQuarantineInfo", GetQuarantineInfo)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
	END_FUNCTION_MAP

	json::value OnBTNCilckClose();
	json::value GetQuarantineInfo();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
	json::value On_GetProductID();
	json::value OnGetAppPath();
};

