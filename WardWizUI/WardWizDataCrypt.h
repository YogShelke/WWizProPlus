/****************************************************
*  Program Name: CWardWizDataCrypt.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 28th March 2016
*  Version No: 2.0.0.1
****************************************************/
#pragma once

#define MAX_MUL_FILE_PATH	MAX_PATH * 4
// CWardWizDataCrypt

class CWardWizDataCrypt :	sciter::event_handler,
							sciter::behavior_factory
{
	HELEMENT self;
public:
	CWardWizDataCrypt();
	virtual ~CWardWizDataCrypt();

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

	BEGIN_FUNCTION_MAP
		FUNCTION_4("OnStartDataEncryption", On_StartDataEncryption) // On_StartDataEncryption()
		FUNCTION_4("OnStartDataDecryption", On_StartDataDecryption) // On_StartDataDecryption()
		FUNCTION_0("ClosePasswordDialog", On_CloseCryptPassDlg) // On_OnCloseCryptPassDlg()	
		FUNCTION_0("StopDataCryptOperation", On_StopDataCryptOperation) // On_OnCloseCryptPassDlg()		
		FUNCTION_2("OpenBrowseWindow", On_OpenBrowseWindow) // On_OpenBrowseWindow()
		FUNCTION_0("OnPauseDataCryptOpr", On_PauseDataCryptOpr) // On_OnCloseCryptPassDlg()		
		FUNCTION_0("OnResumeDataCryptOpr", On_ResumeDataCryptOpr) // On_OnCloseCryptPassDlg()	
		FUNCTION_1("CheckIsNetworkPath", CheckForNetworkPath)
		FUNCTION_1("OnCheckFileExists", OnCheckFileExists)
		FUNCTION_0("CallContinueDataEncDec", On_CallContinueDataEncDec)
		FUNCTION_0("OnSetTimer", OnSetTimer)
	END_FUNCTION_MAP




	//Data Encryption related functions
	json::value On_StartDataEncryption(SCITER_VALUE svStrFilePath, SCITER_VALUE svStrPassword, SCITER_VALUE svBoolKeepOriginal, SCITER_VALUE svFunSetDataCryptStatusCB);
	json::value On_StartDataDecryption(SCITER_VALUE svStrFilePath, SCITER_VALUE svStrPassword, SCITER_VALUE svBoolKeepOriginal, SCITER_VALUE svFunSetDataCryptStatusCB);
	json::value On_CloseCryptPassDlg();
	json::value On_StopDataCryptOperation(); 
	json::value On_OpenBrowseWindow(SCITER_VALUE svBoolSelection, SCITER_VALUE svFunSetFileNameCB);
	json::value On_PauseDataCryptOpr();
	json::value On_ResumeDataCryptOpr();
	json::value CheckForNetworkPath(SCITER_VALUE svFilePath);
	json::value OnCheckFileExists(SCITER_VALUE svFilePath);
	json::value On_CallContinueDataEncDec();
	json::value OnSetTimer();
public:
	TCHAR				m_szFilePath[MAX_MUL_FILE_PATH];
	CString				m_csPassword;
	int					m_iDataOpr;
	bool				m_bManualStop;
	bool				m_bAlreadyEncrypted;
	bool				m_bEncSuccess;
	DWORD				m_dwDecEncStatus;
	CString				m_csEncryptnFilePath;
	bool				m_bIsCryptFinish;
	DWORD				m_dwFinalStatus;
	CString				m_csFinalStatus = L"";
	enum ENUM_EXTERNAL_DEVICE
	{
		INVALID_DRIVE,	
		REMOVABLE_DRIVE,
		CD_DRIVE
	};

	SCITER_VALUE		m_svSetDataCryptStatusCB;
	
	CString BrowseFolderForFileSelection(bool bEncryption, TCHAR *pName, TCHAR *pExt);
	CString ExpandShortcut(CString& csFilename);
	CString  SaveAsDouplicateFile(CString csfilePath);
	
	bool IsPathBelongsToWardWizDir(CString csFilefolderPath);
	int IsPathBelongsFromRemovableDrive(CString csFilefolderPath);
	bool SendDataEncryptionOperation2Service(const TCHAR * chPipeName, DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, DWORD dwValueOperation, bool bDataEncryptionWait);

	void StartCryptOperation(DATACRYPTOPR CryptOperation, TCHAR *csFilePath, TCHAR *chPassword, bool bKeepOriginal);
	void StopCryptOperations(DWORD dwSuccessStatus);
	void ShutDownCryptOperations(bool bManualStop = 0);
	void UpdateDataCryptOpr(LPISPY_PIPE_DATA lpSpyData);
	void SetDataCryptOprStatusOnUI(SCITER_VALUE valAddUpdate, SCITER_STRING strFilePath, SCITER_STRING strFileStatus);
	void OnCallSetCryptStatus(DWORD dwStatus);
};


