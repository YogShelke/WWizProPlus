#pragma once
#include "AdwCleanerConstants.h"
#include "WWizCRC64.h"

typedef enum ENUMTYPE
{
	SERVICESLOC,
	SERVICESDATA,
	FOLDERSLOC,
	FOLDERSDATA,
	FILESLOC,
	FILESDATA,
	SHORTCUTSLOC,
	SHORTCUTSDATA,
	TASKSLOC,
	TASKSDATA,
	REGISTRYLOC,
	REGISTRYDATA,
	BROWSERSFILESLOC,
	BROWSERSFILESDATA,
	BROWSERSREGISTRYLOC,
	BROWSERSREGISTRYDATA
};

const unsigned int MAX_KEY_SIZE = 0x10;
const unsigned int MAX_SIG_SIZE = 0x08;

const unsigned int MAX_VERSIONCHKBUFF = 0x50;
const unsigned int MAX_TOKENSTRINGLEN = 0x04;

const unsigned int MAX_BUFF_SIZE = 0x10 * 0x1000;
const unsigned int MAX_DBVERSIONNO_SIZE = 0x7;

typedef std::vector<string>				VECSTRING;
typedef std::map<CStringA, CStringA>	MAPSYSVARIABLES;

#define ADWDBNAME L"WWIZADWARECLEANER.DB"

class CWWizAdwareScanner
{
public:
	CWWizAdwareScanner();
	virtual ~CWWizAdwareScanner();

	bool LoadDB();
	bool UnLoadDB();
	bool ReLoadDB();
	bool IsFileAlreadyEncrypted(CString csFileName, DWORD &dwDBVersionLength, DWORD &dwDBMajorVersion);
	void ClearVectorData();
	bool ParseDBVersion(LPSTR lpszVersion, DWORD &dwMajorVersion);
	bool LoadContaintFromFile(LPTSTR szFilePath, DWORD dwVersionLength, DWORD &dwSigCount);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	DWORD GetTotalLocationCount();
	DWORD GetTotalDataCount();
	VECSTRING & GetVectorDetails(ENUMTYPE eType);
	DWORD GetServicesLocationCount();
	DWORD GetFoldersLocationCount();
	DWORD GetFilesLocationsCount();
	DWORD GetShortcutsLocationsCount();
	DWORD GetTasksLocationsCount();
	DWORD GetRegLocationsCount();
	DWORD GetBrowserLocationsCount();
	DWORD GetBrowserRegLocationsCount();
	DWORD GetServicesDataCount();
	DWORD GetFoldersDataCount();
	DWORD GetFilesDataCount();
	DWORD GetShortcutsDataCount();
	DWORD GetTasksDataCount();
	DWORD GetRegDataCount();
	DWORD GetBrowserDataCount();
	DWORD GetBrowserRegDataCount();
	bool PrepareSystemPaths();
	bool IsWow64();
private:
	bool GetUsersList();
	bool GetRegistryUsersList();
	void EnumerateSubKeys(HKEY RootKey, char* subKey, unsigned int tabs = 0);
	bool ParseRegExpressionRules(CStringA csInString, VECSTRING &vec2Add);
	bool ParseExpressionRules(CStringA csInString, VECSTRING &vec2Add);
private:
	unsigned long			m_ulFileSize;
	unsigned char			*m_pbyEncDecSig;
	VECSTRING				m_vecServicesLocations;
	VECSTRING				m_vecServicesData;
	VECSTRING				m_vecFoldersLocations;
	VECSTRING				m_vecFoldersData;
	VECSTRING				m_vecFilesLocations;
	VECSTRING				m_vecFilesData;
	VECSTRING				m_vecShortcutsLocations;
	VECSTRING				m_vecShortcutsData;
	VECSTRING				m_vecsTasksLocations;
	VECSTRING				m_vecTasksData;
	VECSTRING				m_vecRegLocations;
	VECSTRING				m_vecRegData;
	VECSTRING				m_vecBrowserLocations;
	VECSTRING				m_vecBrowserData;
	VECSTRING				m_vecBrowserRegLocations;
	VECSTRING				m_vecBrowserRegData;
	VECSTRING				m_vecEmpty;
	VECSTRING				m_vecUsersList;
	VECSTRING				m_vecRegUsersList;
	DWORD					m_dwTotalCount;
	DWORD					m_dwTotalLocCount;
	STRUCTADWCLEANER		m_stAdwCleaner;
	MAPSYSVARIABLES			m_mapSystemPaths;
	bool					m_bIsx64;
};

