#pragma once
#include "stdafx.h"
#include <afxwin.h>
#include "stdafx.h"
//#include "sqlite3.h"

class CWWizParentalCntrl
{
public:
	CWWizParentalCntrl();
	virtual ~CWWizParentalCntrl();
	void StartParentalControl();
	void StartInternetCheck();
	void StartParCtrlCompUsage();
	void StartParCtrlINetAccess();
	enum {
			EN_SUNDAY = 1,
			EN_MONDAY ,
			EN_TUESDAY,
			EN_WEDNESDAY,
			EN_THURSDAY,
			EN_FRIDAY,
			EN_SATURDAY,
	};

	typedef struct _tagParCtrlValCheck{
		int iWDayCompUsage;
		int iWEndCompUsage;
	}PARCTRLUSAGECHECK;


	typedef struct _tagParCtrlINetValCheck{
		int iWDayINetUsage;
		int iWEndINetUsage;
	}PARCTRLINETUSAGECHECK;

	typedef struct _tagParCtrlPermission{
		char szUserName[MAX_PATH];
		bool bIsCompAccessBlocked;
		bool m_bBlockTimePopUpShown;
		bool m_bRestrictHoursPopUp;
		bool m_bBlockInternetUsage;
		bool m_bRestrictInternetUsage;
	}PCTRLUSERPERMISSION;

public:
	typedef std::vector <std::wstring> VECEXEBLOCKLIST;
	typedef std::vector <std::wstring> VECAPPBLOCKLIST;
	typedef std::vector <PCTRLUSERPERMISSION> PCUSERLISTMAP;
	typedef std::map<std::string, bool>MAPUSERLISTCHECK;

	PCUSERLISTMAP					m_vecPCtrlUserList;
	MAPUSERLISTCHECK				m_mapParCtrlUserAccess;
	/*typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
	typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
	typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
	typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
	typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
	typedef  int(*SQLITE3_CLOSE)(sqlite3 *);*/
	/*sqlite3							*m_pdbfile;
	HMODULE							m_hSQLiteDLL;
	SQLITE3_OPEN					m_pSQliteOpen;
	SQLITE3_PREPARE					m_pSQLitePrepare;
	SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
	SQLITE3_STEP					m_pSQLiteStep;
	SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
	SQLITE3_CLOSE					m_pSQLiteClose;*/
	bool							m_bISDBLoaded;
	VECEXEBLOCKLIST					m_vecExeList;
	VECAPPBLOCKLIST					m_vecAppCategoryList;
	bool							m_bBlockInternetUsage;
	bool							m_bRestrictInternetUsage;
	bool							m_bBlockTimePopUpShown;
	bool							m_bRestrictHoursPopUp;
	bool SendParCtrlMessage2Tray(int iRequest, CString csUserName);
	INT64 InsertSQLiteDataForPC(const char* szQuery);
	void LoadExeBlockList();
	bool CheckEXEBlockList(LPTSTR lpszFilePath);
	bool UnloadEXEBlockList(LPTSTR lpszFilePath);
	bool ReLoadBlockedAppList();
	void LoadAppBlockCategoryList();
	bool CheckAppBlockList(LPTSTR lpszFilePath);
	bool UnloadAppBlockList(LPTSTR lpszFilePath);
	CString GetCurrentUsrName();
	bool SendMessage2UI(int iRequest);
	bool CheckEXEBlocked(LPTSTR lpszFilePath);
	void ReLoadBlockedApplication();
	bool IsWardwizPath(LPTSTR lpszFilePath);
	bool CheckIfUserActive(); 
	void LoadUserListforPCtrl();
	void CheckForUserPermissions();
	bool CheckUserAllowed(std::string strUserName);
	bool CheckInternetUsagePermission(CString csUserName);
	void LoadUserPermission();
	bool CheckInternetAccessBlock();
};

