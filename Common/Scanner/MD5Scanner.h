#pragma once

class CMD5Scanner
{
public:
	CMD5Scanner();
	bool LoadMD5Data(FILETYPE, DWORD &dwScan);
	bool UnLoadMD5Data();
	bool ScanFile(LPBYTE lpBuffer, DWORD dwBufferSize, DWORD &VirusID, bool Rescan, FILETYPE filetype = FILE_PE);
	typedef bool(*LoadMD5Database) (FILETYPE, DWORD &dwSigCount);
	typedef bool(*UnLoadMD5Database) ();
	bool LoadReqdLibrary();
	void UnloadLibrary();
	typedef bool(*MD5SCANNER) (LPBYTE pszFilePath, DWORD dwBufferSize, DWORD &dwSpyID, bool bISPEFile, FILETYPE filetype);
	
	virtual ~CMD5Scanner();

public:
	TCHAR						m_szFilePath[MAX_PATH];
	LoadMD5Database				m_loadMD5Data;
	UnLoadMD5Database			m_UnloadMD5Data;
	MD5SCANNER					m_pMD5Scanner;
	HINSTANCE					m_hInstLibrary;
};

