/**********************************************************************************************************
	Program Name			: WardWizCrypter.cpp
	Description				: Wrapper class to encrypt files/folders using AES 256 algorithm.
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 5th Jun 2015
	Version No				: 1.11.0.0
	Special Logic Used		:
	Modification Log		:
***********************************************************************************************************/
#pragma once
#include "_crypto/_fat_aes_cbc.h"

#define FILEBUFFSIZE	8192
#define EXPANDKEYSIZE	240

typedef struct _FILECRYPTOPARAM
{
	TCHAR szFileName[MAX_PATH * 2];
	TCHAR szPassword[33];
	TCHAR szMesDigest5[33];
	DWORD dwCryptVersion;

}WRDWIZ_FILECRYPTOPARAM, *PWRDWIZ_FILECRYPTOPARAM;

class CWardWizCrypter
{
public:
	CWardWizCrypter();
	virtual ~CWardWizCrypter();
public:
	DWORD EncryptFile(	LPCTSTR lpFilePath,
						LPCTSTR csOrgFileName,
						LPCTSTR lpOutFilePath, 
						LPCTSTR lpPassword, 
						void(*_callback)(DWORD dwPercent, void * param),
						void			*callbackParam
						);
	// issue - functionality of saveAs option added.
	// lalit kumawat 7-4-2015
	DWORD DecryptFile(	LPCTSTR lpFilePath, 
						LPCTSTR lpOutFilePath, 
						LPCTSTR lpTempPath,
						BOOL bTmpRequired,
						LPCTSTR lpPassword, 
						void(*_callback)(DWORD dwPercent, void *param),
						void(*_callbackSaveAs)(LPTSTR lpOrgPath, void * param),
						void			*callbackParam
						);
	DWORD Stop();
	DWORD IsFileAlreadyEncrypted(HANDLE hFile);
	CString		m_csOutputFrmDecrpt;
	CString		m_csSaveAsPathDecrpt;
private:
	void Initialize();
	bool GetFileNameFromPath(LPCTSTR lpFilePath, LPTSTR lpOutFileName, DWORD dwOutSize);
	bool VerifyPassword(LPCTSTR szFilePassword, LPCTSTR szUserPassword);

public:
	bool				m_bStopOpr;
	DWORD GetDataEncVersion();
	DWORD GetDataEncVersionSEH();
	DWORD ConvertStringTODWORDSEH(TCHAR* szDataEncVersion);
	DWORD ConvertStringTODWORD(TCHAR* szDataEncVersion);
	bool  ParseVersionNo(LPTSTR lpszVersionNo);
	bool  ParseVersionNoSEH(LPTSTR lpszVersionNo);
	bool CovertDataVersionNoDWORDToString();
	bool CovertDataVersionNoDWORDToString(LPCTSTR lpFileName);
	DWORD CheckFileIntegrityBeforeDecryption(LPCTSTR lpFileName);
	DWORD AddCheckSum(LPCTSTR lpFileName, DWORD dwByteNeedtoAdd);
	DWORD AddCheckSum(LPCTSTR lpFileName);
	CString				m_csDataEncVer;
	CStringArray		m_csArrayDataEncVersion;
	DWORD				m_dwDataEncVersionNo;
	TCHAR				m_szDataEncLogicNo[256];
	TCHAR				m_szDataEncPatchNo[256];
	DWORD				m_dwDataEncVerfromFile;
	TCHAR				m_szDataEncVerfromFile[256];
	DWORD				m_dwFileChecksum;
	BYTE				m_szFileBuffer[FILEBUFFSIZE];;
};

