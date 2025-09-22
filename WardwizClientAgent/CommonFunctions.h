#ifndef COMMONFUNC_H
#define COMMONFUNC_H

#pragma once

//
//#ifdef _USBREGISTRYVALUE
//#undef _USBREGISTRYVALUE
//#endif 

#include "Constants.h"
#include "WardWizDumpCreater.h"
#include "WWizSettingsWrapper.h"
#include "Shlwapi.h"
#include  <SHLOBJ.H>
#include <map>

#ifdef LOG_FILE
#undef LOG_FILE
#endif
#define LOG_FILE					_T("WRDWIZENTERPRIZE.LOG")


#ifdef DISK_SIZE
#undef DISK_SIZE
#endif 



enum class DISK_SIZE : int
{
	EQUAL_SIZE = 1,
	DOUBLE_SIZE = 2,
	TRIPLE_SIZE = 3
};

class CClientAgentCommonFunctions
{
private:
	CString							m_csEmptyDrive;
	ULARGE_INTEGER					m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
	ULARGE_INTEGER					m_uliTotalNumberOfBytes;     // bytes no disco
	ULARGE_INTEGER					m_uliTotalNumberOfFreeBytes; // bytes livres no disco
	DWORD64							m_dwRequiredSpace;
	std::vector<CString>			m_vcsListOfFilePath;
	std::vector<TCHAR>				m_vcsListOfDrive;
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
	//CZipArchive						m_zip;
public:
	CClientAgentCommonFunctions();
	~CClientAgentCommonFunctions();
	//void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);

	bool IsPathWardWizDirPath(CString csFilefolderPath);
	bool IsPathOSReservDirectory(CString csFilefolderPath);

	void GetAllSystemDrive();
	TCHAR GetDefaultDrive(CString csFilePath);
	DWORD EnumerateAllFolderAndFiles(CString m_csaTokenEntryFilePath[], std::vector<CString> &vecFileList);
	CString ExpandShortcut(CString& csFilename);

	bool GetSelectedFileSize(TCHAR *pFilePath, DWORD64 &dwFileSize);
	DWORD64 GetTotalNumberOfFreeBytes(void);
	bool IsRequiredSpaceAvailableOnDrive(TCHAR szDrive, CString csFilePath, int iSpaceRatio);

	//void GetEnviormentVariablesForMachine();
	void GetEnviormentVariablesForMachine(std::map<CString, CString> &mpEnvironmentVars);

	DWORD Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel);
	DWORD GetLoggingLevel4mRegistry();
	bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
	static std::string get_string_from_wsz(const wchar_t* pwsz);
};

#endif