// WardWizOffLineAct.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <stdlib.h>

#include "WardWizOffLineAct.h"

#include "WardWizCRC32.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLEXPORT extern "C" __declspec(dllexport)



DWORD GenerateInstallationCode(LPCTSTR lpSerialNumber, LPCTSTR lpMID, LPTSTR lpInstallCode );
DWORD Get2BytesFromMID(LPCTSTR lpMID, DWORD dwMIDLen, LPBYTE lp2MIDBytes );
DWORD EncryptBuffer(LPVOID lpInData, DWORD dwInSize, LPVOID lpOutData, DWORD &dwOutSize );
DWORD DecryptBuffer(LPVOID lpInData, DWORD dwInSize/*, LPVOID lpOutData, DWORD &dwOutSize*/ );
//DWORD GetAllInfoFromInstallationCode(LPCTSTR lpInstallCode, LPTSTR lpSerialKey, int &iMID1, int &iMID2 );
DWORD GenerateSMSResponse(int iDate, int iMonth, int iYear, int iDaysLeft, int iMID1, int iMID2, BYTE bProductID, LPTSTR lpActivationCode );
DWORD GenerateSMSResponseSEH(int iDate, int iMonth, int iYear, int iDaysLeft, int iMID1, int iMID2, BYTE bProductID, LPTSTR lpActivationCode );
DWORD GetDaysFromSMSResponse( LPBYTE lpSMSResponse, DWORD dwResSize, SYSTEMTIME &ServetTime, WORD &wwDaysLeft );

DWORD ValidateResponseSEH(LPCTSTR lpActivationCode, BYTE bProductID, SYSTEMTIME &ServetTime, WORD &wwDaysLeft );

//DWORD GetDaysLeftFromSerialKey(LPCTSTR lpSerialNumber, BYTE &bProductID, int &iiDaysLeft );
//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CWardWizOffLineActApp

BEGIN_MESSAGE_MAP(CWardWizOffLineActApp, CWinApp)
END_MESSAGE_MAP()


// CWardWizOffLineActApp construction

CWardWizOffLineActApp::CWardWizOffLineActApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	ZeroMemory(m_sz2MIDBytes, sizeof(m_sz2MIDBytes) );

	m_bKey[0] = 0x4D;
	m_bKey[1] = 0x1A;
	m_bKey[2] = 0x6F;
	m_bKey[3] = 0x75;
	m_bKey[4] = 0xB0;
	m_bKey[5] = 0x8C;
	m_bKey[6] = 0xEB;
	m_bKey[7] = 0x9E;
}


// The one and only CWardWizOffLineActApp object

CWardWizOffLineActApp theApp;


// CWardWizOffLineActApp initialization

BOOL CWardWizOffLineActApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


/***************************************************************************************************                
*  Function Name  : GetInstallationCode()
*  Description    : Returns Installation Code at client side.
*  Author Name    : Vilas
*  Modified By	  : Adil Sheikh
*  SR.NO		  : WRDWIZOFFLINEACT_01
*  Date  		  :	20 - Oct - 2014
*  Modified Date  : 03 - Aug - 2016
*  Serial Number   : 12 digits or 12 Hexadecimal chars or 6 Bytes ( ex. 5A8C874G254W or 123456789011 )
****************************************************************************************************/
DLLEXPORT DWORD GetInstallationCode(LPCTSTR lpSerialNumber, LPCTSTR lpMID, LPTSTR lpInstallationCode)
{
	DWORD	dwRet = 0x00;
	TCHAR	szInstallCode[0x20] = {0};
	TCHAR   szNumber[0x11] = { 0 };
	__try
	{
		//AddLogEntry( lpSerialNumber );

		if( IsBadWritePtr(lpInstallationCode, 0x20) )
		{
			dwRet = 0x07 ;
			goto Cleanup ;
		}

		memcpy(&szNumber, lpSerialNumber, 0x10 * sizeof(TCHAR));
		//szNumber[wcslen(szNumber) + 1] = '\0';

		if (0x0C != wcslen(szNumber))
		{
			dwRet = 0x08 ;
			goto Cleanup ;
		}

		/*if( (!bProductID) && (bProductID>0x04) )
		{
			dwRet = 0x09 ;
			goto Cleanup ;
		}*/

		ZeroMemory(theApp.m_sz2MIDBytes, sizeof(theApp.m_sz2MIDBytes) );
		ZeroMemory(szInstallCode, sizeof(szInstallCode));

		dwRet = GenerateInstallationCode(szNumber, lpMID, szInstallCode);
		/*if( 0x1E == wcslen(szInstallCode) )
		{*/
			swprintf_s(lpInstallationCode, 0x1D, L"%s", szInstallCode);
			lpInstallationCode[0x1C] = '\0';
			//dwRet = 0x0A;
			//AddLogEntry(L"### Failed in GetInstallationCode", 0, 0, true, SECONDLEVEL);
		/*}
		else
		{
			dwRet = 0x0A ;
			AddLogEntry(L"### Failed in GetInstallationCode", 0, 0, true, SECONDLEVEL);
		}*/

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x0B;
		AddLogEntry(L"### Exception in GetInstallationCode", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;
}


/***************************************************************************************************                
*  Function Name  : GetAllInfoFromInstallationCode()
*  Description    : Extracts Serial Key, MID bytes from Installation Code at server side
*  Author Name    : Vilas
*  Modified By	  : Adil Sheikh
*  SR.NO		  : WRDWIZOFFLINEACT_02
*  Date  		  :	20 - Oct - 2014
*  Date Modified  :	03 - Aug - 2016
* SerialKey		  : Out Parameter, Serial key must be 12 chracters( 0x06 Bytes)
* iMID1           : Out Parameter, First Byte of MID
* iMID2			  : Out Parameter, Second Byte of MID 
****************************************************************************************************/
DLLEXPORT DWORD GetAllInfoFromInstallationCode(LPTSTR lpInstallCode, LPTSTR lpSerialKey, int &iMID1, int &iMID2 )
//DLLEXPORT DWORD GetAllInfoFromInstallationCode(LPCTSTR lpInstallCode, LPTSTR lpSerialKey, BYTE &iMID1, BYTE &iMID2)
{
	DWORD	dwRet = 0x00, i=0x00, index = 0x00;
	TCHAR	szTemp[0x08] = {0};
	TCHAR	szInstallCode[0x20] = {0};

	BYTE	bInstallCode[0x20] = {0};

	__try
	{	
		if (lpInstallCode == NULL)
		{
			return 0;
		}
		//Installation code must be 16 characters( 0x10 Bytes)
		if( IsBadReadPtr(lpInstallCode, 0x10) )
		{
			AddLogEntry(L"### Failed in IsBadWritePtr, Installation Code: %s ::GetAllInfoFromInstallationCode", lpInstallCode, 0, true, SECONDLEVEL);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( IsBadWritePtr(lpSerialKey, 0x0D) )
		{
			AddLogEntry(L"### Failed in IsBadWritePtr, Serial Key: %s ::GetAllInfoFromInstallationCode", lpSerialKey, 0, true, SECONDLEVEL);
			dwRet = 0x02;
			goto Cleanup ;
		}

		/*if( 0x10 != wcslen(lpInstallCode) )
		{
			dwRet = 0x03;
			goto Cleanup;
		}*/
		//AddLogEntry(lpInstallCode, 0, 0, true, SECONDLEVEL);
		DWORD	dwContent = 0x00;

		index = 0x00;

		_tcsupr(lpInstallCode); //toUpparCase

		//unicode to BYTE format
		for (i = 0x00; i<wcslen(lpInstallCode);)
		{
			szTemp[0x00] = lpInstallCode[i];
			szTemp[0x01] = lpInstallCode[i+1];
			szTemp[0x02] = '\0';

			dwContent = 0x00;
			swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD) );

			bInstallCode[index++] = static_cast < BYTE > (dwContent);

			i += 0x02;
		}

		//for (i = 0x00; i<wcslen(lpInstallCode);)
		//{
			//AddLogEntry(L"Hello %s", (LPCWSTR)i, 0, true, SECONDLEVEL);
			/*szTemp[0x00] = lpInstallCode[i];
			szTemp[0x01] = lpInstallCode[i+1];
			szTemp[0x02] = '\0';

			dwContent = 0x00;
			swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD) );

			bInstallCode[index++] = static_cast < BYTE > (dwContent);

			i += 0x02;*/

			/*if ((lpInstallCode[i] > 0x46) && (lpInstallCode[i] < 0x5B))
			{
				bInstallCode[index++] = static_cast < BYTE > (lpInstallCode[i]);

				i += 0x01;
			}
			else
			{
				if ((lpInstallCode[i] < 0x47))
				{
					if ((lpInstallCode[i + 1]))
					{
						if ((lpInstallCode[i + 1] <= 0x46))
						{
							szTemp[0x00] = lpInstallCode[i];
							szTemp[0x01] = lpInstallCode[i + 1];
							szTemp[0x02] = '\0';

							swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
							bInstallCode[index++] = static_cast <BYTE> (dwContent);

							i += 0x02;
						}
						else
						{
							szTemp[0x00] = lpInstallCode[i];
							szTemp[0x01] = '\0';
							swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
							bInstallCode[index++] = static_cast <BYTE> (dwContent);

							i += 0x01;
						}
					}
					else
					{
						szTemp[0x00] = lpInstallCode[i];
						szTemp[0x01] = '\0';
						swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
						bInstallCode[index++] = static_cast < BYTE > (dwContent);

						i += 0x01;
					}
				}*/
				/*szTemp[0x00] = lpSerialNumber[i];
				szTemp[0x01] = lpSerialNumber[i + 1];
				szTemp[0x02] = '\0';*/

				/*swscanf_s(szTemp, L"%d", &dwContent, sizeof(DWORD));
				szInstallCode[index++] = static_cast < BYTE > (dwContent);

				i += 0x02;*/

			//}
		//}

		dwRet = DecryptBuffer(bInstallCode, index);
		if( dwRet )
		{
			AddLogEntry(L"### Failed in DecryptBuffer ::GetAllInfoFromInstallationCode", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04;
			goto Cleanup;
		}

		//wcscpy_s(szInstallCode, sizeof(szInstallCode), L"");

		for (i = 0x00; i<index; i++)
		{
			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf_s(szTemp, _countof(szTemp), L"%.2X", bInstallCode[i]);
			wcscat_s(szInstallCode, 0x1F, szTemp );
		}
		szInstallCode[0x1D] = '\0';
		AddLogEntry(szInstallCode, 0, 0, true, SECONDLEVEL);
		swprintf_s(lpSerialKey, 0x1D, L"%s", &szInstallCode[0x02]);
		lpSerialKey[0x0C] = '\0';
		AddLogEntry(lpSerialKey, 0, 0, true, SECONDLEVEL);
		iMID1 = bInstallCode[0x00];
		iMID2 = bInstallCode[index - 0x01];
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		dwRet = 0x05;
		AddLogEntry(L"### Exception in GetAllInfoFromInstallationCode", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}


/***************************************************************************************************                
*  Function Name  : GetDaysLeftFromSerialKey()
*  Description    : Checks the serial key for productID. Returns days left if found.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_03
*  Date  		  :	18 - Oct - 2014
****************************************************************************************************/
DLLEXPORT DWORD GetDaysLeftFromSerialKey(LPCTSTR lpSerialNumber, BYTE &bProductID, int &iiDaysLeft )
{
	DWORD	dwRet = 0x00, i=0x00;

	__try
	{

		//

		switch( bProductID )
		{
			case 0x01:
				iiDaysLeft = 365;
				break;

			case 0x02:
				iiDaysLeft = 365*1;
				break;

			case 0x03:
				iiDaysLeft = 365*1;
				break;

			case 0x04:
				iiDaysLeft = 365*1;
				break;

			default:
				iiDaysLeft = 0x00;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		dwRet = 0x04;
		AddLogEntry(L"### Exception in GetDaysLeftFromSerialKey", 0, 0, true, SECONDLEVEL);

		goto Cleanup;
	}

Cleanup :

	return dwRet;
}

/***************************************************************************************************                
*  Function Name  : GetActivationCode()
*  Description    : Returs Activation Code for a gien Installation Code
*  Author Name    : Vilas
*  Modified By	  : Adil Sheikh                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_04
*  Date  		  :	20 - Oct - 2014
*  Date Modified  :	03 - Aug - 2016
*  InstallCode	  : In Parameter, Installation code must be 16 characters( 0x10 Bytes)
* ActivationCode  : Out Parameter, Activation Code generated at server side, to be send to client through SMS
****************************************************************************************************/
DLLEXPORT DWORD GetActivationCode(LPCTSTR lpInstallationCode, int iiDaysLeft, int iMID1, int iMID2, 
								  BYTE bProductID, LPTSTR lpActivationCode)
{

	DWORD	dwRet = 0x00;
	TCHAR	szSerialKey[0x10] = {0};

	__try
	{
		//AddLogEntry(L"### GetActivationCode::1", 0, 0, true, SECONDLEVEL);
		if( IsBadReadPtr(lpInstallationCode, 0x10) )
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//Total length should be 27 (ex 22-1234-5678-1234-5678-8765 )
		if( IsBadWritePtr(lpActivationCode, 0x20) )
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		/*if( wcslen(lpInstallationCode) != 0x10 )
		{
			dwRet = 0x03;
			goto Cleanup;
		}*/

		if( (!bProductID) /*|| (bProductID>0x04) */)
		{
			dwRet = 0x04;
			goto Cleanup;
		}

/*		int		iMID1 = 0x00, iMID2 = 0x00;
		int		iiDaysLeft = 0x00;


		//AddLogEntry(L"### GetActivationCode::3", 0, 0, true, SECONDLEVEL);
		//AddLogEntry(lpInstallationCode, 0, 0, true, SECONDLEVEL);

		dwRet = GetAllInfoFromInstallationCode(lpInstallationCode, szSerialKey, iMID1, iMID2 );
		if( dwRet )
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		//AddLogEntry(L"### GetActivationCode::7", 0, 0, true, SECONDLEVEL);

		BYTE	bProductID = 0x01;

		dwRet = GetDaysLeftFromSerialKey(szSerialKey, bProductID, iiDaysLeft );
		if( dwRet )
		{
			dwRet = 0x06;
			goto Cleanup;
		}
*/

		SYSTEMTIME	ServerTime = {0};

		GetSystemTime( &ServerTime );

		dwRet = GenerateSMSResponseSEH(	ServerTime.wDay, ServerTime.wMonth, ServerTime.wYear, 
										iiDaysLeft, iMID1, iMID2, bProductID, lpActivationCode );
		if( dwRet )
		{
			dwRet = 0x07;
			goto Cleanup;
		}

		//AddLogEntry(L"### GetActivationCode::12", 0, 0, true, SECONDLEVEL);

	}
	//catch(...)
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x08;
		AddLogEntry(L"### Exception in GetActivationCode", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;

}


/***************************************************************************************************                
*  Function Name  : ValidateResponse()
*  Description    : Checks the given Activation code is for this MID and if yes, returns Server date and Number of Days Left.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_05
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DLLEXPORT DWORD ValidateResponse(LPCTSTR lpActivationCode, BYTE bProductID, SYSTEMTIME &ServetTime, WORD &wwDaysLeft )
{

	DWORD	dwRet = 0x00;

	__try
	{
		dwRet = ValidateResponseSEH(lpActivationCode, bProductID, ServetTime, wwDaysLeft );
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x08;
		AddLogEntry(L"### Exception in GetActivationCode", 0, 0, true, SECONDLEVEL);

		goto Cleanup;
	}

Cleanup:

	return dwRet;

}


/***************************************************************************************************                
*  Function Name  : ValidateResponseSEH()
*  Description    : Checks the given Activation code is for this MID and if yes, returns Server date and Number of Days Left.
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_06
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DWORD ValidateResponseSEH(LPCTSTR lpActivationCode, BYTE bProductID, SYSTEMTIME &ServetTime, WORD &wwDaysLeft )
{

	DWORD	dwRet = 0x00, i=0x00, index = 0x00;

	TCHAR	szTemp[0x08] = {0};

	BYTE	szSMSResponse[0x10] = {0};

	try
	{
		if( IsBadReadPtr(lpActivationCode, 0x16) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( 0x16 != wcslen(lpActivationCode) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		DWORD	dwSMSContent = 0x00;

		//Make SMS response of unicode to BYTE format
		for(i=0x00; i<0x16; )
		{
			szTemp[0x00] = lpActivationCode[i];
			szTemp[0x01] = lpActivationCode[i+1];
			szTemp[0x02] = '\0';

			dwSMSContent = 0x00;
			swscanf_s(szTemp, L"%X", &dwSMSContent, sizeof(DWORD));

			szSMSResponse[index++] = static_cast < BYTE > (dwSMSContent);

			i += 0x02;
		}

		//Decrypt Response
		dwRet = DecryptBuffer(szSMSResponse, index );
		if( dwRet )
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		if( bProductID != szSMSResponse[0x09] )
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		DWORD	dwCRC32 = 0x00;
		DWORD	dwSize = 0x08;

		CWardWizCRC32	objCRC32;

		dwRet = objCRC32.GetCheckSum(szSMSResponse, dwSize, dwCRC32);
		if( dwRet )
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		BYTE	bCRC[0x02] = {0};

		bCRC[0] = static_cast < BYTE > ( ((dwCRC32 & 0x0000FF00) >> 0x08) + ((dwCRC32 & 0xFF000000) >> 0x18) );
		bCRC[1] = static_cast < BYTE > ((dwCRC32 & 0x000000FF) + ((dwCRC32 & 0x00FF0000) >> 0x10) );

		if( (bCRC[0] != szSMSResponse[0x08]) || (bCRC[1] != szSMSResponse[0x0A]) )
		{
			dwRet = 0x06;
			goto Cleanup;
		}

		dwRet = GetDaysFromSMSResponse( szSMSResponse, 0x08, ServetTime, wwDaysLeft );
		if( dwRet )
		{
			dwRet = 0x07;
			goto Cleanup;
		}

		//swprintf(lpDateDay, 29, L"Date:%d-%d-%d,Days:%d", ServetTime.wDay, ServetTime.wMonth, ServetTime.wYear, wwDaysLeft );

	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		dwRet = 0x08;
		AddLogEntry(L"### Exception in ValidateResponse", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;

}

/***************************************************************************************************                
*  Function Name  : GenerateInstallationCode()
*  Description    : Returns Installation Code for a given Serial Key and MID
*  Author Name    : Vilas
*  Modified By	  : Adil Sheikh                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_07
*  Date  		  :	20 - Oct - 2014
*  Modified Date  :	03 - Aug - 2016
****************************************************************************************************/
DWORD GenerateInstallationCode(LPCTSTR lpSerialNumber, LPCTSTR lpMID, LPTSTR lpInstallCode )
{
	DWORD	dwRet = 0x00, i=0x00;

	DWORD	dwSerialLen = 0x00, dwMIDLen = 0x00;

	__try
	{
		dwSerialLen = static_cast < DWORD > ( wcslen(lpSerialNumber) );
		if( dwSerialLen != 0x0C )
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwMIDLen = static_cast < DWORD > ( wcslen(lpMID) );
		if( dwMIDLen < 0x04 )
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		if( IsBadWritePtr(lpInstallCode, 0x10) )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		//BYTE	sz2MIDBytes[0x04] = {0};
		BYTE	szInstallCode[0x20] = {0};

		TCHAR	szTemp[0x12] = {0};
		DWORD	index = 0x00, dwContent = 0x00;


		dwRet = Get2BytesFromMID( lpMID, dwMIDLen, theApp.m_sz2MIDBytes );
		if( dwRet )
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		index = 0x01;
		for(i=0x00; i<0x0C; )
		{
			/*if ((lpSerialNumber[i] <= 0x0F))
			szTemp[0x00] = lpSerialNumber[i];
			szTemp[0x01] = lpSerialNumber[i + 1];
			szTemp[0x02] = '\0';*/

			dwContent = 0x00;

			// This logic is used to convert registration key to to 1 BYTE by comparing the range and making their pairing

				if ((lpSerialNumber[i] > 0x46) && (lpSerialNumber[i] < 0x5B))
				{
					szInstallCode[index++] = static_cast < BYTE > (lpSerialNumber[i]);

					i += 0x01;
				}
				else
				{
					if ((lpSerialNumber[i] < 0x47))
					{ 
						if ((lpSerialNumber[i + 1]))
						{
							if ((lpSerialNumber[i + 1] <= 0x46))
							{
								szTemp[0x00] = lpSerialNumber[i];
								szTemp[0x01] = lpSerialNumber[i + 1];
								szTemp[0x02] = '\0';

								swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
								szInstallCode[index++] = static_cast <BYTE> (dwContent);

								i += 0x02;
							}
							else
							{
								szTemp[0x00] = lpSerialNumber[i];
								szTemp[0x01] = '\0';
								swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
								szInstallCode[index++] = static_cast <BYTE> (dwContent);

								i += 0x01;
							}
						}
						else
						{
							szTemp[0x00] = lpSerialNumber[i];
							szTemp[0x01] = '\0';
							swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
							szInstallCode[index++] = static_cast < BYTE > (dwContent);

							i += 0x01;
						}
					}
						/*szTemp[0x00] = lpSerialNumber[i];
						szTemp[0x01] = lpSerialNumber[i + 1];
						szTemp[0x02] = '\0';*/

					/*swscanf_s(szTemp, L"%d", &dwContent, sizeof(DWORD));
					szInstallCode[index++] = static_cast < BYTE > (dwContent);

					i += 0x02;*/

				}
			//swscanf(szTemp, L"%X", &szInstallCode[index++] );

			/*if( (lpSerialNumber[i] > 0x46) &&
				(lpSerialNumber[i] < 0x5B)
				)
				dwContent = static_cast < BYTE > (lpSerialNumber[i]);
			else
			swscanf_s(szTemp, L"%X", &dwContent, sizeof(DWORD));
			szInstallCode[index++] = static_cast < BYTE > (dwContent);*/

			//i += 0x01;
		}

		szInstallCode[0x00] = theApp.m_sz2MIDBytes[0x00];
		//szInstallCode[0x07] = bProductID;
		szInstallCode[index] = theApp.m_sz2MIDBytes[0x01];
		szInstallCode[index + 0x01] = 0x00;

		BYTE	szEncData[0x20] = {0};
		DWORD	dwEncryptedDataSize = 0x00;

		dwRet = EncryptBuffer(szInstallCode, index + 0x01, szEncData, dwEncryptedDataSize);

		if( dwRet )
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		// lpInstallCode length will vary according to hexadecimals provided in registration key

		for (i = 0x00; i<dwEncryptedDataSize; i++)
		{
			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf_s(szTemp, _countof(szTemp), L"%.2X", szEncData[i]);
			wcscat_s(lpInstallCode, 0x1E, szTemp );
		}

		lpInstallCode[0x1D] = '\0';
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		dwRet = 0x06;
		AddLogEntry(L"### Exception in GenerateInstallationCode", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

/***************************************************************************************************                
*  Function Name  : Get2BytesFromMID()
*  Description    : Gets 2 bytes MID Bytes from a given MID string
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_08
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DWORD Get2BytesFromMID(LPCTSTR lpMID, DWORD dwMIDLen, LPBYTE lp2MIDBytes )
{

	DWORD	dwRet = 0x00, i=0x01;

	DWORD	dwValue = 0x00, dwTemp = 0x00;

	BYTE	b1, b2, b3, b4;

	__try
	{

		if( IsBadReadPtr(lpMID, dwMIDLen) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( IsBadWritePtr(lp2MIDBytes, 0x03) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		b1 = b2 = b3 = b4 = 0x00;

		for(; i<dwMIDLen-1; i++ )
		{
			dwTemp = (lpMID[i] << 0x10) + (lpMID[i + 1] >> 0x02) + dwMIDLen;

			dwValue += dwTemp;
		}

		b1 = dwValue>>24;
		b2 = (dwValue<<0x08)>>24;
		b3 = (dwValue<<0x10)>>24;
		b4 = (dwValue<<24)>>24;

		lp2MIDBytes[0] = b2 + b3;
		lp2MIDBytes[1] = b1 + b4;
		lp2MIDBytes[2] = 0x00;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		dwRet = 0x02;
		AddLogEntry(L"### Exception in Get2BytesFromMID", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

/***************************************************************************************************                
*  Function Name  : EncryptBuffer
*  Description    : Encrypt a given buffer and returns in lpOutData parameter of size dwOutSize
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_09
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DWORD EncryptBuffer(LPVOID lpInData, DWORD dwInSize, LPVOID lpOutData, DWORD &dwOutSize )
{
	DWORD	dwRet = 0x00, i=0x00;

	LPBYTE	lpbInData = NULL;
	LPBYTE	lpbOutData = NULL;

	__try
	{


		if( IsBadReadPtr(lpInData, dwInSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( IsBadWritePtr(lpOutData, dwInSize) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( dwInSize <= 0x00 )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		lpbInData = static_cast <LPBYTE> (lpInData);
		lpbOutData = static_cast <LPBYTE> (lpOutData);


		if( !lpbInData )
		{
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		if( !lpbOutData )
		{
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		for( ; i<dwInSize; i++ )
		{
			BYTE	bEncByte = lpbInData[i];
			BYTE	bInter = 0x00;
			DWORD	ulNumRounds = 0x08;

			for(DWORD ulRound = 0x00; ulRound < ulNumRounds; ulRound++)
			{
				bEncByte += (((bInter << 4) ^ (bInter >> 5)) + bInter) ^ (bInter + theApp.m_bKey[ulRound]);

				bInter++;
			}

			lpbOutData[i] = bEncByte;
		}

		dwOutSize = i;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		dwRet = 0x06;
		AddLogEntry(L"### Exception in EncryptBuffer", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}


/***************************************************************************************************                
*  Function Name  : DecryptBuffer()
*  Description    : Exported function, used to decrypt given buffer of size dwInSize using fixed Encryption key
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_10
*  Date  		  :	09 - Sept - 2014
****************************************************************************************************/
DWORD DecryptBuffer(LPVOID lpInData, DWORD dwInSize/*, LPVOID lpOutData, DWORD &dwOutSize*/ )
{
	DWORD	dwRet = 0x00, i=0x00;

	LPBYTE	lpbInData = NULL;

	__try
	{

		if( IsBadWritePtr(lpInData, dwInSize) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( dwInSize <= 0x00 )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		lpbInData = static_cast <LPBYTE> (lpInData);

		if( !lpbInData )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		for( ; i<dwInSize; i++ )
		{
			BYTE	bEncByte = lpbInData[i];
			BYTE	bInter = 0x00;
			DWORD	ulNumRounds = 0x08;

			for(DWORD ulRound = 0x00; ulRound < ulNumRounds; ulRound++)
			{
				bEncByte -= (((bInter << 4) ^ (bInter >> 5)) + bInter) ^ (bInter + theApp.m_bKey[ulRound]);

				bInter++;
			}

			lpbInData[i] = bEncByte;
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		dwRet = 0x04;
		AddLogEntry(L"### Exception in DecryptBuffer", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}


/***************************************************************************************************                
*  Function Name  : GenerateSMSResponseSEH()
*  Description    : Calls GenerateSMSResponse() for Activation Code generation
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_11
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DWORD GenerateSMSResponseSEH(int iDate, int iMonth, int iYear, int iDaysLeft, int iMID1, int iMID2, BYTE bProductID, LPTSTR lpActivationCode )
{
	DWORD	dwRet = 0x00;

	__try
	{
		dwRet = GenerateSMSResponse( iDate, iMonth, iYear, iDaysLeft, iMID1, iMID2, bProductID, lpActivationCode );
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	
	{
		dwRet = 0x08;
		AddLogEntry(L"### Exception in GenerateSMSResponseSEH", 0, 0, true, SECONDLEVEL);

		goto Cleanup;
	}

Cleanup :

	return dwRet;
}

/***************************************************************************************************                
*  Function Name  : GenerateSMSResponse()
*  Description    : Generates Activation Code at server side for given MID and Product ID
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_12
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DWORD GenerateSMSResponse(	int iDate, int iMonth, int iYear, int iDaysLeft, 
							int iMID1, int iMID2, BYTE bProductID, LPTSTR lpActivationCode )
{
	DWORD	dwRet = 0x00;

	CWardWizCRC32	objCRC32;

	try
	{

		if( (iDate > 31) || (iDate<=0x00) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( (iMonth>12) || (iMonth<=0x00) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if( /*(iYear>2019) || */(iYear<2014) )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		if( (iDaysLeft>1095) || (iDaysLeft<=0x00) )
		{
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		if( iMID1 > 0xFF )
		{
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		if( iMID2 > 0xFF )
		{
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		//MID2Y3 X4Y2 X3D1 Y4M1 MID3X1 M2MID1 D2Y1 X2MID4

		BYTE	bSMSResponse[0x10] = {0};
		BYTE	bEncSMSResponse[0x10] = {0};

		DWORD	dwEncSize = 0x00, dwCRC32=0x00;

		bSMSResponse[0x00] = ( (iMID1&0x0000000F)<<0x04 ) + ( (iYear&0x000000F0)>>0x04 );
		bSMSResponse[0x01] = ( (iDaysLeft&0x0000000F)<<0x04 ) + ( (iYear&0x00000F00)>>0x08 );
		bSMSResponse[0x02] = ( iDaysLeft&0x000000F0 ) + ( (iDate&0x000000F0)>>0x04 );
		bSMSResponse[0x03] = ( (iYear&0x0000000F)<<0x04 ) + ( (iMonth&0x00000F0)>>0x04 );
		bSMSResponse[0x04] =  (iMID2&0x000000F0) + ( (iDaysLeft&0x0000F000)>>0x0C );
		bSMSResponse[0x05] = ( (iMonth&0x0000000F)<<0x04 ) + ( (iMID1&0x000000F0)>>0x04 );
		bSMSResponse[0x06] = ( (iDate&0x0000000F)<<0x04 ) + ( (iYear&0x0000F000)>>0x0C );
		bSMSResponse[0x07] = ( (iDaysLeft&0x00000F00)>>0x04 ) + ( iMID2&0x0000000F );


		dwEncSize = 0x08;
		dwRet = objCRC32.GetCheckSum(bSMSResponse, dwEncSize, dwCRC32);

		BYTE	bCRC[0x04] = {0};

		bCRC[0] = static_cast < BYTE > ( dwCRC32 & 0x000000FF) ;
		bCRC[1] = static_cast < BYTE > ( (dwCRC32 & 0x0000FF00) >> 0x08);
		bCRC[2] = static_cast < BYTE > ((dwCRC32 & 0x00FF0000) >> 0x10);
		bCRC[3] = static_cast < BYTE > ((dwCRC32 & 0xFF000000) >> 0x18);

		bSMSResponse[0x08] = bCRC[1] + bCRC[3];
		bSMSResponse[0x09] = bProductID; 
		bSMSResponse[0x0A] = bCRC[0] + bCRC[2];

		//AddLogEntry(L"### GetActivationCode::10", 0, 0, true, SECONDLEVEL);
		dwEncSize = 0x00;
		dwRet = EncryptBuffer( bSMSResponse, 0x0B, bEncSMSResponse, dwEncSize );
		if( dwRet )
		{
			dwRet = 0x07 ;
			goto Cleanup ;
		}

		TCHAR	szTemp[0x08] = {0};

		wcscpy_s(lpActivationCode, sizeof(TCHAR) * 0x16, L"");
		for(int i=0x00; i<0x0B; i++ )
		{
			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf(szTemp, 0x03, L"%.2X", bEncSMSResponse[i] );
			wcscat_s(lpActivationCode, sizeof(TCHAR) * 0x16, szTemp);
		}

		//AddLogEntry(L"### GetActivationCode::11", 0, 0, true, SECONDLEVEL);

		lpActivationCode[0x16] = '\0';

		//AddLogEntry(lpActivationCode, 0, 0, true, SECONDLEVEL);

	}
	//__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	catch( ... )
	{
		dwRet = 0x08;
		AddLogEntry(L"### Exception in GenerateSMSResponse", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return dwRet;
}

/***************************************************************************************************                
*  Function Name  : GetDaysFromSMSResponse()
*  Description    : Parses the Offline Registration Data via SMS and extracts Server Date and Number of Days Left
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_13
*  Date  		  :	20 - Oct - 2014
****************************************************************************************************/
DWORD GetDaysFromSMSResponse( LPBYTE lpSMSResponse, DWORD dwResSize, SYSTEMTIME &ServetTime, WORD &wwDaysLeft )
{

	DWORD	dwRet = 0x00;

	__try
	{
		if( IsBadReadPtr(lpSMSResponse, 0x08) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		if( dwResSize != 0x08 )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		//16 characters( 8 bytes) are in the form of :
		//MID2Y3 X4Y2 X3D1 Y4M1 MID3X1 M2MID1 D2Y1 X2MID4

		BYTE	bMID1 = ( (lpSMSResponse[0x05]&0x0F)<<0x04 ) + ( lpSMSResponse[0x00]>>0x04 );

		if( bMID1 != theApp.m_sz2MIDBytes[0x00] )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		BYTE	bMID2 = (lpSMSResponse[0x04]&0xF0) + (lpSMSResponse[0x07]&0x0F);

		if( bMID2 != theApp.m_sz2MIDBytes[0x01] )
		{
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		//MID2Y3 X4Y2 X3D1 Y4M1 MID3X1 M2MID1 D2Y1 X2MID4
		BYTE	bDate = ( (lpSMSResponse[0x02]&0x0F)<<0x04 ) + ( (lpSMSResponse[0x06]&0xF0)>>0x04 );

		if( (bDate > 0x1F) ||(!bDate) )
		{
			dwRet = 0x05 ;
			goto Cleanup ;
		}


		BYTE	bMonth = ( (lpSMSResponse[0x03]&0x0F)<<0x04 )+ ( (lpSMSResponse[0x05]&0xF0)>>0x04 );

		if( (bMonth > 0x0C) ||(!bMonth) )
		{
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		WORD	wYear = ( (lpSMSResponse[0x06]&0x0F)<<0x0C ) +
						( (lpSMSResponse[0x01]&0x0F)<<0x08 ) +
						( (lpSMSResponse[0x00]&0x0F)<<0x04 ) +
						( (lpSMSResponse[0x03]&0xF0)>>0x04 );

		//Checking 2019 for either user modified SMS response or invalid response
		if( /*(wYear > 2019) ||*/(wYear<2014) )
		{
			dwRet = 0x07 ;
			goto Cleanup ;
		}

		//MID2Y3 X4Y2 X3D1 Y4M1 MID3X1 M2MID1 D2Y1 X2MID4

		WORD	X1 = (WORD) ( (lpSMSResponse[0x04]&0x0F)<<0x0C ) ;
		WORD	X2 = (WORD) ( ((lpSMSResponse[0x07]&0xF0)>>0x04 )<<0x08 );
		BYTE	X3 = (BYTE)( (lpSMSResponse[0x02]&0xF0) );
		BYTE	X4 = (BYTE)( (lpSMSResponse[0x01]&0xF0)>>0x04 );

		WORD wDaysLeft = X1 + X2 + X3 + X4 ;

		//Checking 1095, max 3yrs for either user modified SMS response or invalid response
		if( (wDaysLeft > 1095) ||(!wDaysLeft) )
		{
			dwRet = 0x08 ;
			goto Cleanup ;
		}

		wwDaysLeft = wDaysLeft;

		ServetTime.wDay = bDate;
		ServetTime.wMonth = bMonth;
		ServetTime.wYear = wYear;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	//catch( ... )
	{
		AddLogEntry(L"### Exception in GetDaysFromSMSResponse", 0, 0, true, SECONDLEVEL);
		dwRet = 0x09 ;
	}

Cleanup :

	return dwRet;
}


/***************************************************************************************************                
*  Function Name  : CheckMachineBytes()
*  Description    : Comapres given 2 MID bytes with 2 MID bytes extracted from MID at server side
*  Author Name    : Vilas                                                                                   
*  SR.NO		  : WRDWIZOFFLINEACT_13
*  Date  		  :	19 - Nov - 2014
****************************************************************************************************/
DLLEXPORT bool CheckMachineBytes( LPCTSTR lpMID, DWORD dwMIDLen, int iMID1, int iMID2 )
{
	bool	bRet = true;

	BYTE	bMID[0x04] = {0};

	__try
	{
		if( Get2BytesFromMID( lpMID, dwMIDLen, bMID ) )
		{
			AddLogEntry(L"### Failed in CheckMachineBytes::Get2BytesFromMID", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		if( iMID1 != bMID[0] )
		{
			AddLogEntry(L"### Failed in CheckMachineBytes::MID1 mismatched", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		if( iMID2 != bMID[1] )
		{
			AddLogEntry(L"### Failed in CheckMachineBytes::MID2 mismatched", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		bRet = false;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CheckMachineBytes", 0, 0, true, SECONDLEVEL);
	}

Cleanup :

	return bRet;
}


/*
DLLEXPORT DWORD GetInstallationCodeA(char *lpInstallationCode, char *iDaysLeft, char *lpActivationCode)
{

	MessageBoxA(NULL, lpInstallationCode, "First", 0 );

	MessageBoxA(NULL, iDaysLeft, "Second", 0 );


	int i = strlen( lpInstallationCode );
	int ii = strlen( iDaysLeft);

	if( IsBadWritePtr(lpActivationCode, i+ii+1 ) )
	{
		MessageBoxA(NULL, "Failed to concat because less destination buffer", "Third", 0 );

		return 1;
	}

	strcpy(lpActivationCode, lpInstallationCode );
	strcat(lpActivationCode, iDaysLeft );

	MessageBoxA(NULL, lpActivationCode, "Fourth", 0 );

	return 0;
}

*/