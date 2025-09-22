#pragma once

typedef DWORD (*ADDREGISTRATIONDATA)	(LPBYTE, DWORD, DWORD dwResType, TCHAR *pResName  ) ;

class CITinRegDataOperations
{
public:
	CITinRegDataOperations(void);
	virtual ~CITinRegDataOperations(void);

	bool LoadITinRegDLL();
	bool UnLoadITinRegDLL();
	bool InsertDataIntoDLL( LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName );
	DWORD AddRegistrationDataInFile( LPBYTE lpData, DWORD dwSize);
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
public:
	HMODULE	m_RegisterationDLL ;
	ADDREGISTRATIONDATA	m_AddRegisteredData;
};

