#pragma once
#include "IWardwizSystemTools.h"
#include "RegOptStruct.h"
#include "CommonFunctions.h"
#include "WardWizCrypter.h"
#include "WardWizCompressor.h"
#include "Constants.h"
#include "ISpyCommunicator.h"
#include "iSpyMemMapClient.h"

class CWardwizSystemTools :
	public IWardwizSystemTools
{

private:
	HANDLE							m_hRegOptThread;
	unsigned char					*m_pbyEncDecKey;
	unsigned char					*m_pbyEncDecSig;
	CString							m_csFileFolderPath;
	CString							m_szInputFilePath;
	CString							m_szOutputFilePath;
	CString							m_csOrgFileName;			// to keep org. file name of file to be ecrypted.
	CString							m_csOrgFileNamePath;
	CString							m_csPassword;
	bool							m_bIsFileFromOSPath;
	bool							m_bIsEncrypt;
	std::vector <TCHAR>				m_vcsListOfDrive;
	std::vector <CString>			m_vcsListOfFilePath;
	CClientAgentCommonFunctions		m_objCommonFuncs;
	CWardWizCrypter					m_objWardWizCrypter;
	CWardWizCompressor				m_CWardWizCompressor;
	iSpyServerMemMap_Client			m_objiTinServerMemMap_Client;
	CString							m_csProgramData;
	CString							m_csWindow;
	CString							m_csProgramFile;
	CString							m_csProgramFilex86;
	CString							m_csAppData4;
	CString							m_csDriveName;
	CString							m_csArchitecture;
	CString							m_csBrowseFilePath;
	CString							m_csUserProfile;
	CString							m_csAppData;
	CString							m_csModulePath;
	CString							m_csCurntArchTmpFilePath;
	CString							m_csCurntEncFilePath;
	CString							m_csFinalOutputFilePath;
	CString							m_csSelectedFilePath;
	DWORD							m_dwTotalFiles;
	DWORD							m_dwFilesProcessed;
	DWORD							m_dwKeepOriginal;

	DWORD			DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	DWORD			ReadKeyFromEncryptedFile(HANDLE hFile);
	DWORD			IsFileAlreadyEncrypted(HANDLE hFile);
	DWORD			IsFileAlreadyEncrypted(CString csFilePath);
	bool			IsTempFileSpaceAvailableOnDrive();
	bool			GetSystemTempPath(CString csFileFolderPath, CString &csOutTmpPath);
	DWORD			IsFileSizeMorethen3G(CString csFilePath);
	bool			GetSelectedFileSize(TCHAR *pFilePath, DWORD64 &dwFileSize);
	bool			IsWardwizFile(CString csFilefolderPath);

public:
	CWardwizSystemTools();
	
	virtual ~CWardwizSystemTools();

	BOOL StartRegistryOptimizer() override;

	BOOL StopRegistryOptimizer() override;

	BOOL PauseRegistryOptimizer() override;

	DWORD Encrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus) override;

	DWORD Decrypt_File(TCHAR *m_szFilePath, DWORD &dwStatus) override;

	BOOL EncryptSingleFile(CString csFilePath, CString csOutputPath, CString csOrgFileName) override;

	BOOL DecryptSingleFile(CString csFilePath, CString csTmpPath, bool bTempRequired) override;

	void GetDWORDFromScanOptions(DWORD &dwRegScanOpt, LPREGOPTSCANOPTIONS lpRegOpt);
};

