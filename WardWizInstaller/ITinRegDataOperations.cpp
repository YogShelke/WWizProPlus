#include "stdafx.h"
#include "AVRegInfo.h"
#include "ITinRegDataOperations.h"

//const unsigned int WRDWIZ_KEY = 0x5757495A;			//WWIZ

CITinRegDataOperations::CITinRegDataOperations(void):
	m_RegisterationDLL(NULL)
{
}

CITinRegDataOperations::~CITinRegDataOperations(void)
{
}

bool CITinRegDataOperations::LoadITinRegDLL()
{
	DWORD	dwRet = 0x00 ;
	m_RegisterationDLL = NULL;
	try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		if(!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### Failed to GetModulePath in CVibraniumRegDataOperations::LoadGenXRegDLL", 0, 0, true, SECONDLEVEL);
			return false;
		}

		CString	strRegisterDLL("") ;
		CString	strEvalRegDLL("") ;

		CString	strAppPath = szModulePath;

		strEvalRegDLL.Format( TEXT("%s\\VBEVALREG.DLL"), strAppPath ) ;
		if( !PathFileExists( strEvalRegDLL) )
		{
			dwRet = 0x01 ;
			AddLogEntry(L"VBEVALREG.DLL not found.", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		strRegisterDLL.Format( TEXT("%s\\VBREGISTERDATA.DLL"), strAppPath ) ;
		if( !PathFileExists( strRegisterDLL) )
		{
			dwRet = 0x02 ;
			AddLogEntry(L"VBREGISTERDATA.DLL not found.", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		m_RegisterationDLL = LoadLibrary( strRegisterDLL ) ;
		m_AddRegisteredData = (ADDREGISTRATIONDATA) GetProcAddress(m_RegisterationDLL, "AddRegisteredData") ;

		if( !m_AddRegisteredData )
		{
			dwRet = 0x03 ;
			AddLogEntry(L"VBREGISTERDATA.DLL version is incorrect", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegDataOperations::LoadVibraniumRegDLL", 0, 0, true, SECONDLEVEL);
	}
	return true;

Cleanup:
	return false;
}

bool CITinRegDataOperations::InsertDataIntoDLL( LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName )
{
	try
	{
		if(!LoadITinRegDLL())
		{
			AddLogEntry(L"### Failed to Load VBEVALREG.DLL in CVibraniumRegDataOperations::InsertDataIntoDLL", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if( !m_AddRegisteredData )
		{
			return false;
		}

		if( m_AddRegisteredData(lpResBuffer, dwResSize, dwResType, pResName) != 0 )
		{
			return false;
		}

		if(!UnLoadITinRegDLL())
		{
			AddLogEntry(L"### Failed to UnLoad VBEVALREG.DLL in CVibraniumRegDataOperations::InsertDataIntoDLL", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegDataOperations::InsertDataIntoDLL", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

bool CITinRegDataOperations::UnLoadITinRegDLL()
{
	if(m_RegisterationDLL != NULL)
	{
		FreeLibrary(m_RegisterationDLL);
		m_RegisterationDLL = NULL;
	}
	return true;
}

DWORD CITinRegDataOperations::AddRegistrationDataInFile( LPBYTE lpData, DWORD dwSize)
{
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		AddLogEntry(L"### Faile to GetModulePath in CWardwizRegDataOperations::AddRegistrationDataInFile", 0, 0, true, SECONDLEVEL);
		return false;
	}

	CString	strUserRegFile = szModulePath ;
	strUserRegFile = strUserRegFile + L"\\VBUSERREG.DB" ;

	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	DWORD	dwRet = 0x00, dwBytesWrite ;

	AVACTIVATIONINFO	ActInfo = {0} ;

	hFile = CreateFile(	strUserRegFile, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;

	if( hFile == INVALID_HANDLE_VALUE )
	{
		AddLogEntry(L"### VBUSERREG.DB failed to create in CITinRegDataOperations::AddRegistrationDataInFile", 0, 0, true, SECONDLEVEL);
		dwRet = 0x01 ;
		goto Cleanup ;
	}
	else
	{
		AddLogEntry(L"### VBUSERREG.DB created successfully in CITinRegDataOperations::AddRegistrationDataInFile", 0, 0, true, SECONDLEVEL);
	}

	memcpy(&ActInfo, lpData, dwSize ) ;

	if( DecryptData( (LPBYTE )&ActInfo, dwSize) )
	{
		dwRet = 0x02 ;
		goto Cleanup ;
	}

	dwBytesWrite = 0x00 ;
	
	WriteFile( hFile, &ActInfo, dwSize, &dwBytesWrite, NULL ) ;
	if( dwSize != dwBytesWrite )
		dwRet = 0x03 ;

Cleanup:

	if( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile ) ;
	hFile = INVALID_HANDLE_VALUE ;


	return dwRet ;
}

DWORD CITinRegDataOperations::DecryptData( LPBYTE lpBuffer, DWORD dwSize )
{
	if( IsBadWritePtr( lpBuffer, dwSize ) )
		return 1 ;

	DWORD	iIndex = 0 ;
	DWORD jIndex = 0;

	if (lpBuffer == NULL || dwSize == 0x00)
	{
		return 1;
	}

	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
	{
		if(lpBuffer[iIndex] != 0)
		{
			if((lpBuffer[iIndex] ^ (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE)) == 0)
			{
				lpBuffer[iIndex] = lpBuffer[iIndex];
			}
			else
			{
				lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex] + WRDWIZ_KEYSIZE);
				jIndex++;
			}
			if (jIndex == WRDWIZ_KEYSIZE)
			{
				jIndex = 0x00;
			}
			if (iIndex >= dwSize)
			{
				break;
			}
		}
	}

	//DWORD	i = 0 ;
	//DWORD dwEncKey = WRDWIZ_KEY;

	//if (lpBuffer == NULL || dwSize == 0x00)
	//{
	//	return 1;
	//}

	//for (i = 0x00;  i < dwSize; i+= 4)
	//{
	//	if(*((DWORD *)&lpBuffer[i]) != 0x0)
	//	{
	//		dwEncKey += LOWORD(dwEncKey);
	//		*((DWORD *)&lpBuffer[i]) = *((DWORD *)&lpBuffer[i]) ^ dwEncKey ;
	//	}
	//}
	return 0;
}