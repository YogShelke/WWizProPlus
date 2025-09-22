//WardWizRestartClient.h  :Header File For the Restart Client Try Pop Up
//
#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"
#include "windows.h"
#include "sqlite3.h"

typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
typedef  int(*SQLITE3_CLOSE)(sqlite3 *);

//WardWizRestartClient Dialog

class CWardWizRestartClient :public CDialog,
	public CSciterBase,
	public sciter::host<CWardWizRestartClient>
{
	DECLARE_DYNAMIC(CWardWizRestartClient);
	HELEMENT self;

public:
	CWardWizRestartClient(CWnd* pParent = NULL);  // Standard constructor
	virtual ~CWardWizRestartClient();
	
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

// Dialog Data
	enum { IDD = IDD_DIALOG_RESTART_LOGOFF };

protected:
	virtual  void DoDataExchange(CDataExchange* pDX);	// DDX/DDV Support

	DECLARE_MESSAGE_MAP();

public:
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedOK();

	virtual BOOL OnInitDialog();
	int GetTaskBarWidth();
	int GetTaskBarHeight();
	void OnBnClickedClose();
	void HideAllElements();
	void SetParentWindow();
	sciter::dom::element m_root_el;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);

	int m_iTrayMsgType;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnClickOK", On_ClickOK);
		FUNCTION_0("GetDBPath", GetDBPath);
		FUNCTION_0("OnClickClose", On_ClickClose);
		FUNCTION_1("OnCheckMsgType", OnCheckMsgType);
	END_FUNCTION_MAP

	SCITER_VALUE m_svGetMsgType;

	json::value GetDBPath();
	//json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	json::value On_ClickOK();
	json::value On_ClickClose();
	json::value OnCheckMsgType(SCITER_VALUE svGetMsgType);

private:
	bool LoadRequiredLibrary();

private:
	sqlite3							*m_pdbfile;
	HMODULE							m_hSQLiteDLL;
	SQLITE3_OPEN					m_pSQliteOpen;
	SQLITE3_PREPARE					m_pSQLitePrepare;
	SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
	SQLITE3_STEP					m_pSQLiteStep;
	SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
	SQLITE3_CLOSE					m_pSQLiteClose;
	bool							m_bISDBLoaded;
};