#pragma once

#include "stdafx.h"
#include "afxwin.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"
#include <vector>

// CWWizDetectQuarentineDlg dialog
typedef struct __tagThreatInfor
{
	TCHAR  szThreatName[MAX_PATH];
	TCHAR  szFilePath[MAX_PATH];
	DWORD   dwActionTaken;
}THREATINFO;

typedef std::vector<THREATINFO> VECTHREATINFO;

class CWWizDetectQuarentineDlg : public CDialog, public CSciterBase,
	public sciter::host<CWWizDetectQuarentineDlg>
{
	DECLARE_DYNAMIC(CWWizDetectQuarentineDlg)

public:
	CWWizDetectQuarentineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWWizDetectQuarentineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TRAY_POPUP_QUARNTINE };
	
	virtual BOOL OnInitDialog();
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	int GetTaskBarWidth();
	int GetTaskBarHeight();

public:
	sciter::dom::element			m_root_el;

	int								m_iCurrentIndex;
	VECTHREATINFO					m_vecThreatList;
	bool							bIsNextPrevClicked;
	int								m_iSize;
	bool							m_bFirstEntry;
	bool							m_bIsCloseClicked;
	HELEMENT						self;
	HANDLE							m_hTrayPoupUpStopEvent;

public :
	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

public:
	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetQuarantineInfo", GetQuarantineInfo)
		//FUNCTION_0("OnClickNext",OnBTNClickNext)
		//FUNCTION_0("OnClickPrev", OnBTNClickPrev)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("OnBtnClose", OnBTNCilckClose)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_1("OnGetShortPath", On_GetShortPath)
	END_FUNCTION_MAP

	json::value GetQuarantineInfo();
	//json::value OnBTNClickNext();
	//json::value OnBTNClickPrev();
	json::value OnBTNCilckClose();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetThemeID();
	json::value On_GetShortPath(SCITER_VALUE svPath);

	bool AddIntoQueue(CString csThreatName, CString csFilePath, DWORD dwActionTaken);
	//void ClearEntries();
	bool ISDuplicateEntry(CString csFilePath);
	void AddQuarantineInfo(CString, CString, int, int);

	json::value On_GetLanguageID();
	//void UpdateDialog(CString csThreatName, CString csThreatPath, int ActionTaken,int  m_iSize);

};
