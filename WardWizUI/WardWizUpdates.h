/**********************************************************************************************************
Program Name          : WardWizLiveUpdate.cpp
Description           : This class contains the functionality for updating product from local folder.
It has 2 options
1) Update from internet.
2) Update from Local Folder.
Author Name			  : Nihar Deshpande
Date Of Creation      : 27th May 2016
Version No            : 2.0.0.14
Special Logic Used    :

Modification Log      :
1. Nihar           Created WardWizLiveUpdate class   27th May 2016
***********************************************************************************************************/
#pragma once

#include "DownloadConts.h"
#include "WardWizLiveUpdate.h"
#include "iSpyMemMapClient.h"

typedef DWORD(*UNZIPFILE)			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);
typedef bool(*STOPUNRAROPERATION)   (void);

class CWardWizUpdates : sciter::event_handler,
						sciter::behavior_factory
{
public:
	CWardWizUpdates();
	~CWardWizUpdates();

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

	BEGIN_FUNCTION_MAP
		//Updates relatedd functions
		FUNCTION_2("OnClickUpdatesFromLocalFolder", On_ClickUpdatesFromLocalFolder)//On_UpdatesNextButton
		FUNCTION_5("OnClickUpdateFromInternet", On_ClickUpdateFromInternet)//On_ClickUpdateFromInternet
		FUNCTION_0("OnPauseUpdates", On_PauseUpdates)//On_PauseResumeUpdates
		FUNCTION_0("OnResumeUpdates", On_ResumeUpdates)//On_PauseResumeUpdates
		FUNCTION_2("OnClickStopUpdates", On_ClickStopUpdates)//On_ClickStopUpdates
		FUNCTION_1("SendNotificationMessage", On_SendNotificationMessageCB)
		FUNCTION_0("OnPauaseOfflineUpdate", On_PauaseOfflineUpdate);
		FUNCTION_0("OnResumeOfflineUpdate", On_ResumeOfflineUpdate);
		FUNCTION_0("OnCloseOfflineUpdate", On_CloseOfflineUpdate);
		FUNCTION_1("SendUpdateType", Send_UpdateType);

	END_FUNCTION_MAP

	//Updates related functions
	json::value On_ClickUpdatesFromLocalFolder(SCITER_VALUE svFilePath, SCITER_VALUE svNotificationMessageFromLocalCB);
	json::value On_ClickUpdateFromInternet(SCITER_VALUE m_pSetUpdateStatusCb, SCITER_VALUE m_pAddUpdateTableCb, SCITER_VALUE m_pUpdateUpdateTableCb, SCITER_VALUE m_pRowAddCb, SCITER_VALUE m_pUpdateCompleteCb);
	json::value On_PauseUpdates();
	json::value On_ResumeUpdates();
	json::value On_ClickStopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop);
	json::value On_SendNotificationMessageCB(SCITER_VALUE svFunNotificationMessageCB);
	json::value On_PauaseOfflineUpdate();
	json::value On_ResumeOfflineUpdate();
	json::value On_CloseOfflineUpdate();
	json::value Send_UpdateType(SCITER_VALUE m_csUpdateType);

	eLIVE_UPDATE_TYPE					m_updateType;
	CWardWizLiveUpdate					m_objWardWizLiveUpdate;
	CStringArray						m_csFilesList;
	iSpyServerMemMap_Client				m_objIPCALUpdateClient;

	int									m_iTotalNoofFiles;
	INT64								m_iUpdateID;

	bool								m_bOlderdatabase;
	bool								m_bdatabaseuptodate;
	bool								m_bOfflineUpdateStarted;
	bool								m_bAborted;

	bool								LoadExtractDll();
	CString								GetModuleFilePath();

	HMODULE								m_hZip;
	HMODULE								m_hStopUnRarOperation;
	UNZIPFILE							m_UnzipFile;
	STOPUNRAROPERATION					m_StopUnRarOperation;

	HANDLE								m_hUpdateFromLocalFolderThread;

	CString								m_csInputFolderPath;
	CString								m_csVersionNo;
	CString								m_csFileName;
	CString								m_CurrentFilePath;

	DWORD								m_dwMaxVersionInZip;
	DWORD								m_dwCurrentLocalFileSize;

	
	SCITER_VALUE						m_svLiveUpdateStatusFunctionCB;
	SCITER_VALUE						m_svAddUpdateTableCb;
	SCITER_VALUE						m_svUpdateUpdateTableCb;
	SCITER_VALUE						m_svRowAddCb;
	SCITER_VALUE						m_svUpdateCompleteCb;
	SCITER_VALUE						m_svNotificationMessageFromLocalCB;
	SCITER_VALUE						m_svFunDisplayDownloadType;
	SCITER_VALUE						m_svUpdateDownloadTypeCB;
	
	
	void EnumFolder(LPCTSTR pstr);
	void OnBnClickedButtonNext();
	void CallUISetPauseStatusfunction(CString csData);

	bool PauseUpdates();
	bool ResumeUpdates();
	bool ShutDownDownload();
	bool StopUpdates(SCITER_VALUE svIsStopFrmTaskbar, SCITER_VALUE svbIsManualStop);
	bool UpdateVersionIntoRegistry();
	bool CheckForValidUpdatedFiles(CString csInputFolder, std::vector<CString> &csVectInputFiles);
	bool StartDownloading();
	bool StartALUpdateUsingALupdateService();
	bool CreateDirectoryFocefully(LPTSTR lpszPath);
	bool ValidateFileNVersion(CString csFileName, CString csVersion, DWORD dwDBVersionLength);
	bool EnumAndDeleteTempFolder(CString csInputFileName);
	bool CheckForMaxVersionInZip(CString csVersionNo);

	
	CString ExtractRARFile(CString csInputFileName, DWORD &dwUnzipCount);
	CString GetAppFolderPath();

	DWORD ValidateDB_File(TCHAR *m_szFilePath, DWORD &dwStatus, CString &csInputPathProgramData);
	DWORD CheckForValidVersion(CString csVersionNo);
	DWORD ReadDBVersionFromReg();
	void CallNotificationMessage(int iMsgType, SCITER_STRING strMessageString);
	DWORD SendData2CommService(int iMesssageInfo, bool bWait);
};