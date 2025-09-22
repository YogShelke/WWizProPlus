#pragma once
#include "WardWizDatabaseInterface.h"
#include "WWizAdwareCleaner.h"

class WWizAdwareRecoverDlg : sciter::event_handler,
	sciter::behavior_factory
{
public:
	WWizAdwareRecoverDlg();
	virtual ~WWizAdwareRecoverDlg();
	HELEMENT self;
	
	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {

	}
	virtual void detached(HELEMENT he) {
	}

	void GetRecordsSEH();
	void OnBtnClickRecoverEntries();
	void EmptyDirectory(TCHAR* folderPath, bool bTakeBackup = true);
	bool RemoveFile(CString csPath);
	INT64 InsertDataToTable(const char* szQuery);

	SCITER_VALUE m_svRecoverArrayFn;
	SCITER_VALUE m_svArrayRecover;

	BEGIN_FUNCTION_MAP
		FUNCTION_1("OnLoadRecover", OnLoadRecoverEntries)
		FUNCTION_1("OnClickRecover", OnBtnClickRecover)
	END_FUNCTION_MAP

	json::value OnLoadRecoverEntries(SCITER_VALUE svRecoverArray);
	json::value OnBtnClickRecover(SCITER_VALUE svArrayRecover);
};

