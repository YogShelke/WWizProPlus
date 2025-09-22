/**********************************************************************************************************
Program Name          : WWizSplVirusScan.cpp
Description           : Class to scan special type of viruses
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 14th Mar 2016
Version No            :
Special Logic Used    : This class is created for the scanning of special type of viruses

Modification Log      :
1. Ram Shelke	          : Created Class CWWizSplVirusScan       14 - 03 - 2016
***********************************************************************************************************/
#include "StdAfx.h"
#include "WWizSplVirusScan.h"

CWWizSplVirusScan::CWWizSplVirusScan()
{
}

CWWizSplVirusScan::~CWWizSplVirusScan()
{
}

DWORD CWWizSplVirusScan::ScanFile(LPCTSTR lpFileName, LPTSTR lpVirusName, DWORD &dwSpyID, bool bRescan)
{
	DWORD	dwRet = 0x00;
	HANDLE	hFileHandle = INVALID_HANDLE_VALUE;

	if (!lpFileName)
	{
		return dwRet;
	}

	__try
	{
		hFileHandle = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileHandle == INVALID_HANDLE_VALUE)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		//To check VBE and VBS file present at the root of removable drive
		//Added by Pradip on date 1-08-2018
		DWORD	dwFileSize = GetFileSize(hFileHandle, NULL);
		TCHAR	szVirusName[0x100] = { 0 };
		bool	bVirusFound = false;
		if (dwFileSize)
		{
			bVirusFound = false;
			CheckForVBEVBSAtRemovalDrive(lpFileName, hFileHandle, bVirusFound, szVirusName);
			if (bVirusFound)
			{
				_tcscpy_s(lpVirusName, (MAX_PATH - 1), szVirusName);
				goto Cleanup;
			}

			//Added by Pradip on 12-02-2019 to detect the malicious script files ( Samples provided by Tariq update.js )
			if ((dwFileSize > 0x300) && (dwFileSize < 0x11100))
			{
				if (StrStrI(lpFileName, L".js"))
				{

					CheckForMaliciousJScript(hFileHandle, dwFileSize, bVirusFound, szVirusName);
					if (bVirusFound)
					{
						_tcscpy_s(lpVirusName, (MAX_PATH - 1), szVirusName);
						goto Cleanup;
	    			}
				}
			}
	     }
		
		if ((dwFileSize == 0xFFFFFFFF) || (!dwFileSize) ||
			(dwFileSize > 0x5000) ||				//Thumb.db size		= 8,432 byes
			(dwFileSize < 0xED)						//autorun.inf size	= 237 byes
			)
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		//Modified date : 28 / 02 / 2015
		//Analyzed and Need to do :: if( (dwFileSize > 0xFFF ) && (dwFileSize < 0x2000) )
		//if( (dwFileSize > 0x300 ) && (dwFileSize < 0x2000) )
		if ((dwFileSize > 0x13FF) && (dwFileSize < 0x2000))
		{
			dwRet = 0x03;
			goto Cleanup;
		}
		
		BYTE	szData[0x60] = { 0 };
		DWORD	dwReadBytes = 0x00;

		ReadFile(hFileHandle, szData, 0x60, &dwReadBytes, NULL);
		if (0x60 != dwReadBytes)
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		bVirusFound = false;
		
		CheckForMaliciousHtml(hFileHandle, dwFileSize, szData, dwReadBytes, bVirusFound, szVirusName);
		if (bVirusFound)
		{
			_tcscpy_s(lpVirusName, (MAX_PATH - 1), szVirusName);
			goto Cleanup;
		}


		if (dwFileSize < 0x500)
		{
			//Modified function by pradip on date 30-8-2018
			CheckForAutorunInfFile(szData, dwReadBytes,hFileHandle, bVirusFound);
			if (bVirusFound)
			{
				_tcscpy_s(lpVirusName, MAX_PATH, L"WW.WORM.AUTORUN.INF");
				goto Cleanup;
			}
		}

		//Modified by Vilas on 04 / 12 / 2015
		//due to malware found of size 0x1000( 2088 )
		if ((dwFileSize > 0xFF) && (dwFileSize < 0x5000))
		{
			CheckForAutorunLNKFile(lpFileName, szData, dwReadBytes, hFileHandle, dwFileSize, bVirusFound, szVirusName);
			if (bVirusFound)
			{
				if (wcslen(szVirusName))
				{
					_tcscpy_s(lpVirusName, MAX_PATH, szVirusName);
				}
				else
				{
					_tcscpy_s(lpVirusName, MAX_PATH, L"WW.TRO.AUTORUNNER.B");
				}

				goto Cleanup;
			}
		}

		if ((dwFileSize > 0x1FFF) && (dwFileSize < 0x3001))
		{
			CheckForThumbsDBFile(szData, dwReadBytes, hFileHandle, dwFileSize, bVirusFound);
			if (bVirusFound)
			{
				_tcscpy_s(lpVirusName, MAX_PATH, L"WW.WORM.AUTORUN.T");
				goto Cleanup;
			}
		}

		if ((dwFileSize > 0x1FFF) && (dwFileSize < 0x3001))
		{
			CheckForCantix(szData, dwReadBytes, hFileHandle, dwFileSize, bVirusFound);
			if (bVirusFound)
			{
				_tcscpy_s(lpVirusName, MAX_PATH, L"WW.WORM.VBS.CANTIX.A");
				goto Cleanup;
			}
		}

		if ((dwFileSize > 0x3BFF) && (dwFileSize < 0x5001))
		{
			CheckforDesktopVBS(szData, dwReadBytes, hFileHandle, dwFileSize, bVirusFound);
			if (bVirusFound)
			{
				_tcscpy_s(lpVirusName, MAX_PATH, L"WW.WORM.VBS.Dunihi.T");
				goto Cleanup;
			}
		}

		dwRet = 0x05;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = 0x06;
		AddLogEntry(L"### Exception in CWardwizSplVirusScan::CheckFileForViruses, File: %s", lpFileName, 0, true, SECONDLEVEL);
	}
Cleanup:

	if (hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFileHandle);
		hFileHandle = INVALID_HANDLE_VALUE;
	}
	return dwRet;
}

bool CWWizSplVirusScan::CheckForAutorunInfFile(LPBYTE	lpbBuffer, DWORD dwSize, HANDLE hFile, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		//[autorun
		BYTE	bAutorun[0x08] = { 0x5B, 0x61, 0x75, 0x74, 0x6F, 0x72, 0x75, 0x6E };

		if (memcmp(lpbBuffer, bAutorun, sizeof(bAutorun)) == 0x00)
		{
			//open=WScript.exe //e:VBScript 
			BYTE bOpenWScript[0x1D] = { 0x6F, 0x70, 0x65, 0x6E, 0x3D, 0x57, 0x53, 0x63,
				0x72, 0x69, 0x70, 0x74, 0x2E, 0x65, 0x78, 0x65,
				0x20, 0x2F, 0x2F, 0x65, 0x3A, 0x56, 0x42, 0x53,
				0x63, 0x72, 0x69, 0x70, 0x74	/*, 0x20, 0x74, 0x68,
												0x75, 0x6D, 0x62, 0x2E, 0x64, 0x62, 0x20, 0x61,
												0x75, 0x74, 0x6F	*/
			};

			if (memcmp(&lpbBuffer[0x0B], bOpenWScript, sizeof(bOpenWScript)) == 0x00)
				bFound = bVirusFound = true;

		}
		//Added By Pradip on date 30-8-2018

		BYTE	bFileData[0x200] = { 0 };
		DWORD	dwBytesRead = 0x00;

		//execute=windrv.exe.....Miner Trojan Horse (CryptoCurrencies)
		BYTE bExeWindrv[0x12] = { 0x65, 0x78, 0x65, 0x63, 0x75, 0x74, 0x65, 0x3D, 0x77, 0x69, 0x6E, 0x64, 0x72, 0x76, 0x2E, 0x65, 0x78, 0x65 };

		SetFilePointer(hFile, 0x60, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bExeWindrv))
			 goto Cleanup;

		DWORD dwIter = 0x00;
		
		for (; dwIter < dwBytesRead; dwIter++)
		{
			
			if (!bVirusFound)
			{
				if (_memicmp(&bFileData[dwIter], bExeWindrv, sizeof(bExeWindrv)) == 0x00)
				{
					bFound = bVirusFound = true;
					goto Cleanup;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSplVirusScan::CheckForAutorunInfFile", 0, 0, true, SECONDLEVEL);
		bVirusFound = false;
	}

Cleanup:

	return bVirusFound;
}

//Modified for Gamerue 
//by Vilas on 13 / 07 / 2015
bool CWWizSplVirusScan::CheckForAutorunLNKFile(LPCTSTR lpFileName, LPBYTE lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound, LPTSTR lpszVirusName)
{
	bool bWScriptFound = false;
	bool bWThumbsFound = false;

	try
	{
		bFound = false;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		//4C 00 00 00
		if (*((DWORD *)&lpbBuffer[0x00]) != 0x4C)
			goto Cleanup;

		//
		//if ((*((DWORD *)&lpbBuffer[0x50]) != 0x4FE0501F) || (*((DWORD *)&lpbBuffer[0x54]) != 0x3AEA20D0) ||

		if ((((*((DWORD *)&lpbBuffer[0x50])) & 0x4FE0FF1F) != ((*((DWORD *)&lpbBuffer[0x50])) & 0x4FE0FF1F)))
			goto Cleanup;

		 //commented by pradip on 14-12-2017
		/*
		if ((*((DWORD *)&lpbBuffer[0x54]) != 0x3AEA20D0) ||
			(*((DWORD *)&lpbBuffer[0x58]) != 0xD8A21069) ||
			(*((DWORD *)&lpbBuffer[0x5C]) != 0x302B0008)
			)
			goto Cleanup;
		*/

		BYTE	bFileData[0x400] = { 0 };
		DWORD	dwBytesRead = 0x00;
		DWORD	dwIter = 0x00;

		//Added By Pradip on 02-04-2019
		SetFilePointer(hFile, 0x100, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x1D0, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		//Cutommer Narayan 
		//SystemVolumeInformation.exe&start 20160625_151642.jpg.exe
		//3100350031003600340032002E006A00700067002E00650078006500
		BYTE b1542JPGExe[] = { 0x31, 0x00, 0x35, 0x00, 0x31, 0x00, 0x36, 0x00, 0x34, 0x00, 0x32,
			0x00, 0x2E, 0x00, 0x6A, 0x00, 0x70, 0x00, 0x67, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00 };
		
	
		bFound = false;
		dwIter = 0x00;
		dwBytesRead -= 0x20;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (_memicmp(&bFileData[dwIter], b1542JPGExe, sizeof(b1542JPGExe)) == 0x00)
			{
				bFound = true;
				goto Cleanup;
			}

		}

		//Added by Pradip on date 12-04-2019
		//Given by Tarique on 12-4-2019
		//c.m.d...e.x.e
		//K.q.O.e.G.m.G.e.u.K.m.C.a.q.O.i.
		BYTE bCMDExe[] = { 0x63, 0x00, 0x6D, 0x00, 0x64, 0x00, 0x2E, 0x00, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };
		BYTE bKQOEGM[] = { 0x4B, 0x00, 0x71, 0x00, 0x4F, 0x00, 0x65, 0x00, 0x47, 0x00, 0x6D, 0x00,
			               0x47, 0x00, 0x65, 0x00, 0x75, 0x00, 0x4B, 0x00, 0x6D, 0x00, 0x43, 0x00, 
						   0x61, 0x00, 0x71, 0x00, 0x4F, 0x00, 0x69 };

		bFound = bWScriptFound= false;
		dwIter = 0x00;
		dwBytesRead -= 0x14;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCMDExe, sizeof(bCMDExe)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bCMDExe);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bKQOEGM, sizeof(bKQOEGM)) == 0x00)
				{
					bFound = true;
					break;

				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By Pradip on 08-04-2019
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0x100, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		//Cutommer Narayan 
		//SystemVolumeInformation.exe&start 20160625_151642.jpg.exe
		//3100350031003600340032002E006A00700067002E00650078006500
		
		//BYTE b1542JPGExe[] = { 0x31, 0x00, 0x35, 0x00, 0x31, 0x00, 0x36, 0x00, 0x34, 0x00, 0x32,
		//	0x00, 0x2E, 0x00, 0x6A, 0x00, 0x70, 0x00, 0x67, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00 };

		bFound = false;
		dwIter = 0x00;
		dwBytesRead -= 0x20;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (_memicmp(&bFileData[dwIter], b1542JPGExe, sizeof(b1542JPGExe)) == 0x00)
			{
				bFound = true;
				goto Cleanup;
			}

		}

		//Added By Pradip on 08-04-2019
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0x100, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		//\.d.f.r.d.b.i.c.i.\.a.e.c.f.j.b.w.w...e.x.e
		//61006500630066006A006200770077002E006500780065
		BYTE bAECFJBWWExe[] = { 0x61, 0x00, 0x65, 0x00, 0x63, 0x00, 0x66, 0x00, 0x6A, 0x00, 0x62, 0x00, 0x77,
			0x00, 0x77, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		bFound = false;
		dwIter = 0x00;
		dwBytesRead -= 0x1B;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (_memicmp(&bFileData[dwIter], bAECFJBWWExe, sizeof(bAECFJBWWExe)) == 0x00)
			{
				bFound = true;
				goto Cleanup;
			}

		}


		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0x100, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x400, &dwBytesRead, NULL);
		if (dwBytesRead < 0x49)
			goto Cleanup;

		unsigned char bWScriptExe[0x38] = { 0x77, 0x00, 0x73, 0x00, 0x63, 0x00, 0x72, 0x00,
			0x69, 0x00, 0x70, 0x00, 0x74, 0x00, 0x2E, 0x00,
			0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x00, 0x00,
			0x1A, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x2F, 0x00,
			0x2F, 0x00, 0x65, 0x00, 0x3A, 0x00, 0x56, 0x00,
			0x42, 0x00, 0x53, 0x00, 0x63, 0x00, 0x72, 0x00,
			0x69, 0x00, 0x70, 0x00, 0x74, 0x00, 0x20, 0x00	/*,
															0x74, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00,
															0x62, 0x00, 0x2E, 0x00, 0x64, 0x00, 0x62, 0x00, 0x20
															*/
		};


		//Added for Gamerue 
		//by Vilas on 13 / 07 / 2015
		TCHAR	szDrive[0x08] = { 0 };
		TCHAR	szVolumeName[0x100] = { 0 };


		//by Vilas on 07 / 09 / 2015
		TCHAR	szTemp[0x100] = { 0 };
		bFound = false;
		const wchar_t *pFileName = wcsrchr(lpFileName, '\\');

		szDrive[0] = lpFileName[0];
		szDrive[1] = lpFileName[1];
		szDrive[2] = lpFileName[2];

		//Modified by Vilas on 08 / 07 / 2016
		//to support Win10 detection, issue no:0001550
		if ((wcsstr(pFileName, L"GB)."))
			)
		{
			bFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
			goto Cleanup;
		}

		GetVolumeInformationW(szDrive, szVolumeName, 0xFF, NULL, NULL, NULL, NULL, NULL);
		if (wcslen(szVolumeName))
		{
			_wcsupr_s(szVolumeName);

			if (wcsstr(pFileName, szVolumeName))
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
				goto Cleanup;
			}

			//by Vilas on 07 / 09 / 2015
			swprintf_s(szTemp, 255, L" (%c) ", szDrive[0]);

			//if (wcsstr(pFileName, L"GB)."))
			//Modified by Vilas on 07 / 09 / 2015
			if ((wcsstr(pFileName, L"GB).")) ||
				(wcsstr(pFileName, szTemp))
				)
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
				goto Cleanup;
			}
		}

		//by Vilas on 15 / 09 / 2015
		//for the detection of Worm.Win32.Gamarue.lnk
		bFound = false;
		dwIter = 0x08;

		if ((wcslen(pFileName) > 0x09) &&
			(GetDriveType(szDrive) == DRIVE_REMOVABLE)
			)
		{
			for (; dwIter < 0x101;)
			{
				ZeroMemory(szTemp, sizeof(szTemp));
				swprintf_s(szTemp, 255, L"(%luGB).", dwIter);
				if (wcsstr(pFileName, szTemp))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.TRO.AUTORUNNER.AA");
					goto Cleanup;
				}

				dwIter *= 0x02;
			}
		}

		/*bFound = false;
		dwIter = 0x00;
		dwBytesRead -= 0x49;
		for (; dwIter<dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bWScriptExe, 0x15) == 0x00)
					bWScriptFound = true;
			}

			if (!bWThumbsFound)
			{
				if (_memicmp(&bFileData[dwIter + 0x24], &bWScriptExe[0x24], 0x14) == 0x00)
					bWThumbsFound = true;
			}

			if (bWScriptFound && bWThumbsFound)
			{
				bFound = true;
				break;
			}
		}

		if (bFound)
			goto Cleanup;*/


		//Added By Pradip on date 27-03-2019
		//$..\..\..\..\Windows\system32\cmd.exe\/c start http: //new.internet-start.net/?utm_source=beatle^&utm..	
		//750074006D005F0073006F0075007200630065003D0062006500610074006C0065005E002600750074006D005F006D0065006400690075006D003D00690063006F006E005E002600750074006D005F00630061006D0070006100690067006E
		BYTE bInternetStartCheck[] = { 0x75, 0x00, 0x74, 0x00, 0x6D, 0x00, 0x5F, 0x00, 0x73, 0x00, 0x6F, 0x00,
			0x75, 0x00, 0x72, 0x00, 0x63, 0x00, 0x65, 0x00, 0x3D, 0x00, 0x62, 0x00,
			0x65, 0x00, 0x61, 0x00, 0x74, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x5E, 0x00,
			0x26, 0x00, 0x75, 0x00, 0x74, 0x00, 0x6D, 0x00, 0x5F, 0x00, 0x6D, 0x00,
			0x65, 0x00, 0x64, 0x00, 0x69, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x3D, 0x00,
			0x69, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x5E, 0x00, 0x26, 0x00,
			0x75, 0x00, 0x74, 0x00, 0x6D, 0x00, 0x5F, 0x00, 0x63, 0x00, 0x61, 0x00,
			0x6D, 0x00, 0x70, 0x00, 0x61, 0x00, 0x69, 0x00, 0x67, 0x00, 0x6E };

		//%APPDATA%\DRPSu\internet_start.ico
		//5C0041007000700044006100740061005C0052006F0061006D0069006E0067005C00440052005000530075005C0069006E007400650072006E00650074005F00730074006100720074002E00690063006F
		BYTE bINETIcoCheck[] = { 0x5C, 0x00, 0x41, 0x00, 0x70, 0x00, 0x70, 0x00, 0x44, 0x00, 0x61,
			0x00, 0x74, 0x00, 0x61, 0x00, 0x5C, 0x00, 0x52, 0x00, 0x6F, 0x00,
			0x61, 0x00, 0x6D, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x5C,
			0x00, 0x44, 0x00, 0x52, 0x00, 0x50, 0x00, 0x53, 0x00, 0x75, 0x00,
			0x5C, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72,
			0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x5F, 0x00, 0x73, 0x00,
			0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x2E, 0x00, 0x69,
			0x00, 0x63, 0x00, 0x6F };

		
		//if (dwBytesRead < 0x20)
			//goto Cleanup;


		//bFound = bWScriptFound = false;

		//dwIter = 0x00;
		//dwBytesRead -= 0x50;

		/*for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bInternetStartCheck, sizeof(bInternetStartCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bInternetStartCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bINETIcoCheck, sizeof(bINETIcoCheck)) == 0x00)
				{
					bFound = true;
					break;
					
				}
			}
		}

		if (bFound)
		    goto Cleanup;*/


		//Added by Vilas on 31 March 2015
		//Samples given by Tapasvi ( his harddisk )

		//c.m.d...e.x.e.
		//63 00 6d 00 64 00 2e 00 65 00 78 00 65 00

		// /.c s.t.a.r.t.
		//63 00 20 00 73 00 74 00 61 00 72 00  74 00

		//v.b.s.&
		//76 00 62 00 73 00 26 00
		//		or

		//V.B.S.&
		//56 00 42 00 53 00 26 00
		//Found lnk sapmles on 14 April 2015 so used _memicmp instead of memcmp

		BYTE	bCmdExe[] = { 0x63, 0x00, 0x6d, 0x00, 0x64, 0x00, 0x2e, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00 };
		BYTE	bCStart[] = { 0x63, 0x00, 0x20, 0x00, 0x73, 0x00, 0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00 };
		BYTE	bVbsExt[] = { 0x76, 0x00, 0x62, 0x00, 0x73, 0x00, 0x26, 0x00 };

		BYTE	bVBSExt[] = { 0x56, 0x00, 0x42, 0x00, 0x53, 0x00, 0x26, 0x00 };

		DWORD	dwCStartLocation = 0x00;

		bFound = false;
		dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCmdExe, sizeof(bCmdExe)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bCmdExe);
					continue;
				}
			}

			if ((bWScriptFound) && (!bWThumbsFound))
			{
				if (_memicmp(&bFileData[dwIter], bCStart, sizeof(bCStart)) == 0x00)
				{
					bWThumbsFound = true;
					dwIter += sizeof(bCStart);
					dwCStartLocation = dwIter;
					continue;
				}
			}

			//if (bWScriptFound && bWThumbsFound)
			if (dwCStartLocation)
			{
				//(memcmp(&bFileData[dwIter], bVBSExt, sizeof(bVBSExt)) == 0x00))
				if ((_memicmp(&bFileData[dwIter], bVbsExt, sizeof(bVbsExt)) == 0x00) &&
					((dwIter - dwCStartLocation)<0x20)
					)
				{
					bFound = true;
					break;
				}
			}

		}


		if (bFound)
			goto Cleanup;


		//Added by Vilas on 27 April 
		//Samples given by Customer Nilesh


		BYTE	bc[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00 };		//Checking /c(1 space)

		BYTE	bclsstart[29] = { 0x73, 0x00, 0x26, 0x00, 0x63, 0x00, 0x6C, 0x00, 0x73, 0x00, 0x26, 0x00, 0x63, 0x00, 0x6C,
			0x00, 0x73, 0x00, 0x26, 0x00, 0x73, 0x00, 0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74
		};


		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCmdExe, sizeof(bCmdExe)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bCmdExe);
					continue;
				}
			}

			if ((bWScriptFound) && (!bWThumbsFound))
			{
				if (_memicmp(&bFileData[dwIter], bc, sizeof(bc)) == 0x00)
				{
					bWThumbsFound = true;
					dwIter += sizeof(bCStart);
					dwCStartLocation = dwIter;
					continue;
				}
			}

			if (bWScriptFound && bWThumbsFound)
			{
				if ((_memicmp(&bFileData[dwIter], bclsstart, sizeof(bclsstart)) == 0x00) &&
					((dwIter - dwCStartLocation)<0x70)
					)
				{
					bFound = true;
					break;
				}
			}

		}


		if (bFound)
			goto Cleanup;

		//Added by Vilas on 06 April 2015
		//Samples given by Tapasvi ( Anupam Pandey )

		//R.E.C.Y.C.L.E.R.\.S.-.
		//52 00 45 00 43 00 59 00 43 00 4C 00 45 00 52 00 5C 00 53 00 2D 00

		//c.p.l.
		//63 00 70 00 6C 00


		BYTE	bRecycler[] = { 0x52, 0x00, 0x45, 0x00, 0x43, 0x00, 0x59, 0x00, 0x43, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x52, 0x00, 0x5C, 0x00, 0x53, 0x00, 0x2D, 0x00 };
		BYTE	bCpl[] = { 0x63, 0x00, 0x70, 0x00, 0x6C, 0x00 };


		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRecycler, sizeof(bRecycler)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRecycler);
					dwCStartLocation = dwIter;
					continue;
				}
			}

			if ((bWScriptFound) &&
				((dwIter - dwCStartLocation)<0x80)
				)
			{
				if (_memicmp(&bFileData[dwIter], bCpl, sizeof(bCpl)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added by Vilas on 15 April 2015
		//Samples given by Anupam Pandey

		//:\Windows\System32\rundll32.exe
		BYTE bRundll32Path[32] = { 0x3A, 0x5C, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x5C, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D,
			0x33, 0x32, 0x5C, 0x72, 0x75, 0x6E, 0x64, 0x6C, 0x6C, 0x33, 0x32, 0x2E, 0x65, 0x78, 0x65 };

		//(1 spaces)rundll32.exe (2 spaces)
		BYTE bRundll32ExeName[22] = { 0x20, 0x00, 0x72, 0x00, 0x75, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x6C, 0x00, 0x6C, 0x00,
			0x33, 0x00, 0x32, 0x00, 0x20, 0x00, 0x20, 0x00 };

		//shell32.dll
		BYTE bShell32DLLName[22] = { 0x73, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x33, 0x00, 0x32, 0x00, 0x2E,
			0x00, 0x64, 0x00, 0x6C, 0x00, 0x6C, 0x00 };


		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRundll32Path, sizeof(bRundll32Path)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRundll32Path);
					dwCStartLocation = dwIter;

				}

				continue;
			}


			if ((bWScriptFound) && (!bWThumbsFound))
			{
				if (_memicmp(&bFileData[dwIter], bRundll32ExeName, sizeof(bRundll32ExeName)) == 0x00)
				{
					bWThumbsFound = true;
					dwIter += sizeof(bRundll32ExeName);
					dwCStartLocation = dwIter;

				}

				continue;
			}

			if ((bWScriptFound) && (bWThumbsFound) && ((dwIter - dwCStartLocation)<0x10)
				)
			{
				if (_memicmp(&bFileData[dwIter], bShell32DLLName, sizeof(bShell32DLLName)) == 0x00)
				{
					bFound = true;
					break;
				}
			}

		}

		if (bFound)
			goto Cleanup;


		//Added by Vilas on 04 Dec 2015
		//Samples given by Tapi ( Customer is Krishna Ballare)

		//\ & attrib +s +h %cd
		BYTE bAttribSH[41] = { 0x5C, 0x00, 0x20, 0x00, 0x26, 0x00, 0x20, 0x00, 0x61, 0x00, 0x74, 0x00, 0x74, 0x00, 0x72, 0x00,
			0x69, 0x00, 0x62, 0x00, 0x20, 0x00, 0x2B, 0x00, 0x73, 0x00, 0x20, 0x00, 0x2B, 0x00, 0x68, 0x00,
			0x20, 0x00, 0x25, 0x00, 0x63, 0x00, 0x64, 0x00, 0x25 };

		//exe & start %temp%
		BYTE bStartTemp[37] = { 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x20, 0x00, 0x26, 0x00, 0x20, 0x00, 0x73, 0x00, 0x74, 0x00,
			0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x20, 0x00, 0x25, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6D, 0x00,
			0x70, 0x00, 0x25, 0x00, 0x5C };

		bWScriptFound = false;
		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bAttribSH, sizeof(bAttribSH)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bAttribSH);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bStartTemp, sizeof(bStartTemp)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 7-11-2017
		///.c. .".{.b.5.d.c.c.7.f.7.-.e.4.3.f.
		//2F 00 63 00 20 00 22 00 7B 00 62 00 35 00 64 00 63 00 63 00 37 00 66 00 37 00 2D 00 65 00 34 00 33 00 66 00
		BYTE bFirstCheck[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00, 0x22, 0x00, 0x7B, 0x00, 0x62,
			0x00, 0x35, 0x00, 0x64, 0x00, 0x63, 0x00, 0x63, 0x00, 0x37, 0x00,
			0x66, 0x00, 0x37, 0x00, 0x2D, 0x00, 0x65, 0x00, 0x34, 0x00, 0x33,
			0x00, 0x66, 0x00 };

		//2.b.7.4.c.2...e.x.e
		//32 00 62 00 37 00 34 00 63 00 32 00 2E 00 65 00 78 00 65
		BYTE bSecondCheck[] = { 0x32, 0x00, 0x62, 0x00, 0x37, 0x00, 0x34, 0x00, 0x63, 0x00,
			0x32, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };
		

		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bFirstCheck, sizeof(bFirstCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bFirstCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bSecondCheck, sizeof(bSecondCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 9-11-2017
		//Customer Abdul Momin from Gujarat
		//w"..r.u.n.d.l.l.3.2...e.x.e
		//77 22 F7 00 72 00 75 00 6E 00 64 00 6C 00 6C 00 33 00 32 00 2E 00 65 00 78 00 65
		BYTE bRundll32Check[] = { 0x77, 0x22, 0xF7, 0x00, 0x72, 0x00, 0x75,
			0x00, 0x6E, 0x00, 0x64, 0x00, 0x6C, 0x00,
			0x6C, 0x00, 0x33, 0x00, 0x32, 0x00, 0x2E,
			0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		//Modified By Pradip on 10-11-2017
		//rJ]. .rundll32.exe..J.......rJ].rJ].
		// 72 4A 5D A7 20 00 72 75 6E 64 6C 6C 33 32 2E 65 78 65 00 00 4A 00 09 00 04 00 EF BE 72 4A 5D A7 72 4A 5D

		BYTE bRJCheck[] = { 0x72, 0x4A, 0x5D, 0xA7, 0x20, 0x00, 0x72, 0x75, 0x6E,
			0x64, 0x6C, 0x6C, 0x33, 0x32, 0x2E, 0x65, 0x78, 0x65,
			0x00, 0x00, 0x4A, 0x00, 0x09, 0x00, 0x04, 0x00, 0xEF,
			0xBE, 0x72, 0x4A, 0x5D, 0xA7, 0x72, 0x4A, 0x5D };


		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRJCheck, sizeof(bRJCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRundll32Check);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRundll32Check, sizeof(bRundll32Check)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 25-11-2017
		//Modified by pradip on date 9-4-2019 to avoid FP
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0x70, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x100, &dwBytesRead, NULL);

		//xKnM..951360~1
		//784B6E4D10003935313336307E31
		BYTE bXKNMKCheck[] = { 0x78,0x4B,0x6E,0x4D,0x10,0x00,0x39,0x35,0x31,0x33,0x36,0x30,0x7E,0x31 };

		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = false;

		dwIter = 0x00;
		dwBytesRead -= 0x12;

		for (; dwIter < dwBytesRead; dwIter++)
		{
		
			if (_memicmp(&bFileData[dwIter], bXKNMKCheck, sizeof(bXKNMKCheck)) == 0x00)
				{
					bFound = true;
				   goto Cleanup;
			}
		}


		//Added By pradip on 14-12-2017
		//Customer Sachin Bingude from pune
		//R.o.a.m.i.n.g.....f.2
		//52 00 6F 00 61 00 6D 00 69 00 6E 00 67 00 00 00 16 00 66 00 32
		BYTE bRoamingCheck[] = { 0x52, 0x00, 0x6F, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x69,
			0x00, 0x6E, 0x00, 0x67, 0x00, 0x00, 0x00, 0x16, 0x00, 0x66, 0x00, 0x32 };

		//pc-pc...........&.Q.A:"M....
		//70 63 2D 70 63 00 00 00 00 00 00 00 00 00 00 00 26 B1 51 D0 41 3A 22 4D AD CE 80 B2
		BYTE bPCCheck[] = { 0x70, 0x63, 0x2D, 0x70, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00,
			                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0xB1, 0x51, 0xD0,
			                0x41, 0x3A, 0x22, 0x4D, 0xAD, 0xCE, 0x80, 0xB2 };



		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRoamingCheck, sizeof(bRoamingCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRoamingCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bPCCheck, sizeof(bPCCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 4-01-2018
		//Customer sachin
		//G./.c. .s.t.a.r.t.
		//47 00 2F 00 63 00 20 00 73 00 74 00 61 00 72 00 74 00
		BYTE bGCheck[] = { 0x47, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x20,
			                0x00, 0x73, 0x00, 0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00 };

		//&.s.t.a.r.t. .e.x.p.l.o.r.e.r
		//26 00 73 00 74 00 61 00 72 00 74 00 20 00 65 00 78 00 70 00 6C 00 6F 00 72 00 65 00 72
		BYTE bStartCheck[] = { 0x26, 0x00, 0x73, 0x00, 0x74, 0x00, 0x61, 0x00,
			                0x72, 0x00, 0x74, 0x00, 0x20, 0x00, 0x65, 0x00,
							0x78, 0x00, 0x70, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x65, 0x00, 0x72 };



		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bGCheck, sizeof(bGCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bGCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bStartCheck, sizeof(bStartCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 09-01-2018
		//Customer Yogeen patel
		//Modified on date 12-07-2018 by pradip for FP removing
		//h.t.t.p.s.:././.l.a.u.n.c.h.p.a.g.e.
		//680074007400700073003A002F002F006C00610075006E00630068007000610067006500

		BYTE bhttpsCheck[] = { 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x73, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x75, 0x00, 0x6E, 0x00, 0x63, 0x00, 0x68, 0x00, 0x70, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00 };

		//u.i.d.=.o.T.l.K.B.C.j.M.h.x.1.s.X.m.
		//7500690064003D006F0054006C004B00420043006A004D00680078003100730058006D00

		BYTE buidCheck[] = { 0x75, 0x00, 0x69, 0x00, 0x64, 0x00, 0x3D, 0x00, 0x6F, 0x00, 0x54, 0x00, 0x6C, 0x00, 0x4B, 0x00, 0x42, 0x00, 0x43, 0x00, 0x6A, 0x00, 0x4D, 0x00, 0x68, 0x00, 0x78, 0x00, 0x31, 0x00, 0x73, 0x00, 0x58, 0x00, 0x6D, 0x00 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bhttpsCheck, sizeof(bhttpsCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bhttpsCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], buidCheck, sizeof(buidCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 09-01-2018
		//Customer Yogeen patel
		//JE*..Google..:.......qJ.&.JE**
		//4A 45 2A 10 00 47 6F 6F 67 6C 65 00 00 3A 00 08 00 04 00 EF BE 71 4A DD 26 8A 4A 45 2A 2A
		BYTE bJeGoogleCheck[] = { 0x4A, 0x45, 0x2A, 0x10, 0x00, 0x47, 0x6F, 0x6F, 0x67, 0x6C, 0x65, 0x00, 0x00, 0x3A, 0x00, 
			                       0x08, 0x00, 0x04, 0x00, 0xEF, 0xBE, 0x71, 0x4A, 0xDD, 0x26, 0x8A, 0x4A, 0x45, 0x2A, 0x2A };

		//C.h.r.o.m.e.....\.1......J.H..APPLIC
		//43 00 68 00 72 00 6F 00 6D 00 65 00 00 00 16 00 5C 00 31 00 00 00 00 00 A3 4A A5 48 10 00 41 50 50 4C 49 43
		BYTE bChromeCheck[] = {0x43, 0x00, 0x68, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x6D, 0x00, 
			                   0x65, 0x00, 0x00, 0x00, 0x16, 0x00, 0x5C, 0x00, 0x31, 0x00,
							   0x00, 0x00, 0x00, 0x00, 0xA3, 0x4A, 0xA5, 0x48, 0x10, 0x00,
							   0x41, 0x50, 0x50, 0x4C, 0x49, 0x43 };



		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bJeGoogleCheck, sizeof(bJeGoogleCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bJeGoogleCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bChromeCheck, sizeof(bChromeCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;
		
		//Added By pradip on 16-01-2018
		//Customer soham services
		

		///.c. .s.t.a.r.t. .%.C.D.%
		//2F 00 63 00 20 00 73 00 74 00 61 00 72 00 74 00 20 00 25 00 43 00 44 00 25
		BYTE bCStartCheck[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00, 0x73, 0x00, 0x74, 0x00, 
			                    0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x20, 0x00, 0x25, 0x00, 0x43, 0x00, 0x44, 0x00, 0x25 };

		//".n.j.w.0.r.m...e.x.e.
		//22 00 6E 00 6A 00 77 00 30 00 72 00 6D 00 2E 00 65 00 78 00 65 00
		BYTE bnjwormCheck[] = { 0x22, 0x00, 0x6E, 0x00, 0x6A, 0x00, 0x77, 0x00, 0x30,
			0x00, 0x72, 0x00, 0x6D, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00 };


		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCStartCheck, sizeof(bCStartCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bCStartCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bnjwormCheck, sizeof(bnjwormCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 17-01-2018

        //R.2......:.. .cmd.exe.<.
		//52 00 32 00 00 9A 04 00 EE 3A C8 09 20 00 63 6D 64 2E 65 78 65 00 3C 00
		BYTE bR2Check[] = { 0x52, 0x00, 0x32, 0x00, 0x00, 0x9A, 0x04, 0x00,
			                0xEE, 0x3A, 0xC8, 0x09, 0x20, 0x00, 0x63, 0x6D, 
							0x64, 0x2E, 0x65, 0x78, 0x65, 0x00, 0x3C, 0x00 };

		//:..*....N.....
		//3A C5 BA 2A 00 00 00 C6 4E 00 00 00 00 01
		BYTE bNCheck[] = { 0x3A, 0xC5, 0xBA, 0x2A, 0x00, 0x00, 0x00, 0xC6, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x01 };


		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bR2Check, sizeof(bR2Check)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bR2Check);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bNCheck, sizeof(bNCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 3-02-2018

		//H.o.m.e.P.a.g.e.D.e.f.e.n.d.e.r
		//48 00 6F 00 6D 00 65 00 50 00 61 00 67 00 65 00 44 00 65 00 66 00 65 00 6E 00 64 00 65 00 72
		
		BYTE bHomeDefenderCheck[] = { 0x48, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x50,
			                          0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x44, 0x00,
									  0x65, 0x00, 0x66, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72 };

		//S.t.u.f.f...e.x.e.......^
		//53 74 75 66 66 2E 65 78 65 00 40 00
		
		BYTE bStuffCheck[] = { 0x53, 0x74, 0x75, 0x66, 0x66, 0x2E, 0x65, 0x78, 0x65, 0x00, 0x40, 0x00 };


		if (dwBytesRead < 0x20)
			goto Cleanup;

		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bHomeDefenderCheck, sizeof(bHomeDefenderCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bHomeDefenderCheck);
				}

				continue;
			}

			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bStuffCheck, sizeof(bStuffCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 12-02-2018

		//.|....I.J.H..K...2
		//A07C0FCEF30149CC4A8648D5D44B04EF8F32

		BYTE bIJHKCheck[] = { 0xA0, 0x7C, 0x0F, 0xCE, 0xF3, 0x01, 0x49, 0xCC, 0x4A, 0x86, 0x48, 0xD5, 0xD4, 0x4B, 0x04, 0xEF, 0x8F, 0x32 };

		//1SPS..XF.L8C....&.m.m
		//31535053E28A5846BC4C3843BBFC139326986DCE6D

		BYTE b1SPSCheck[] = { 0x31, 0x53, 0x50, 0x53, 0xE2, 0x8A, 0x58, 0x46, 0xBC, 0x4C, 0x38, 0x43, 0xBB, 0xFC, 0x13, 0x93, 0x26, 0x98, 0x6D, 0xCE, 0x6D };


			if (dwBytesRead < 0x20)
				goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bIJHKCheck, sizeof(bIJHKCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bIJHKCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], b1SPSCheck, sizeof(b1SPSCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added By pradip on 12-02-2018

		//1SPS..XF.L8C....&.m
		//31535053E28A5846BC4C3843BBFC139326986D

		BYTE bXFCheck[] = { 0x31, 0x53, 0x50, 0x53, 0xE2, 0x8A, 0x58, 0x46, 0xBC, 0x4C, 0x38, 0x43, 0xBB, 0xFC, 0x13, 0x93, 0x26, 0x98, 0x6D };

		//inder...........03.....D.V..
		//696E64657200000000000000000000003033F7D5E1A3B544AB56B798

		BYTE bInderCheck[] = { 0x69, 0x6E, 0x64, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x33, 0xF7, 0xD5, 0xE1, 0xA3, 0xB5, 0x44, 0xAB, 0x56, 0xB7, 0x98 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bXFCheck, sizeof(bXFCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bXFCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bInderCheck, sizeof(bInderCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 19-02-2018
		//Customer Abdul 
		//Modified on date 12-07-2018 by pradip for FP removing
		//.c. .".{.c.4.5.d.e.d.7.
		//2F006300200022007B006300340035006400650064003700

		BYTE bc45deCheck[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00, 0x22, 0x00, 0x7B, 0x00, 0x63, 0x00, 0x34, 0x00, 0x35, 0x00, 0x64, 0x00, 0x65, 0x00, 0x64, 0x00, 0x37, 0x00 };

		//\.6.2.d.c.d.6.6.3.-.3.b.
		//5C00360032006400630064003600360033002D0033006200

		BYTE b62dcCheck[] = { 0x5C, 0x00, 0x36, 0x00, 0x32, 0x00, 0x64, 0x00, 0x63, 0x00, 0x64, 0x00, 0x36, 0x00, 0x36, 0x00, 0x33, 0x00, 0x2D, 0x00, 0x33, 0x00, 0x62, 0x00 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bc45deCheck, sizeof(bc45deCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bc45deCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], b62dcCheck, sizeof(b62dcCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		         //Added By pradip on 21-02-2018

		//R.2...........cmd.exe.<.
		//5200320000000000000000000000636D642E657865003C00

		BYTE bRCmdCheck[] = { 0x52, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6D, 0x64, 0x2E, 0x65, 0x78, 0x65, 0x00, 0x3C, 0x00 };

			//wN....]N.D...Q.
			//774EC11AE7025D4EB7442EB1AE5198

		BYTE bNNCheck[] = { 0x77, 0x4E, 0xC1, 0x1A, 0xE7, 0x02, 0x5D, 0x4E, 0xB7, 0x44, 0x2E, 0xB1, 0xAE, 0x51, 0x98 };


			if (dwBytesRead < 0x20)
				goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bRCmdCheck, sizeof(bRCmdCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bRCmdCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bNNCheck, sizeof(bNNCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 21-02-2018

		//NE.pnE.m....t.Z.v.j.a.G.l.f.I.P...e.x.e
		//4E4513706E45176D1400000074005A0076006A00610047006C006600490050002E006500780065

		BYTE bNEpneCheck[] = { 0x4E, 0x45, 0x13, 0x70, 0x6E, 0x45, 0x17, 0x6D, 0x14, 0x00, 0x00, 0x00, 0x74, 0x00, 0x5A, 0x00, 0x76, 0x00, 0x6A, 0x00, 0x61, 0x00, 0x47, 0x00, 0x6C, 0x00, 0x66, 0x00, 0x49, 0x00, 0x50, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		//T...........*.gF.....D:\
		//5400000011000000030000002A8867461000000000443A5C

		BYTE bGFCheck[] = { 0x54, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x2A, 0x88, 0x67, 0x46, 0x10, 0x00, 0x00, 0x00, 0x00, 0x44, 0x3A, 0x5C };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bNEpneCheck, sizeof(bNEpneCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bNEpneCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bGFCheck, sizeof(bGFCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;
		
		//Added By pradip on 22-02-2018
		//Customer Yogesh Gavali
		//:.. .cmd.exe.<........:.
		//3AC8092000636D642E657865003C0008000400EFBEED3AC5

		BYTE bcmdCheck[] = { 0x3A, 0xC8, 0x09, 0x20, 0x00, 0x63, 0x6D, 0x64, 0x2E, 0x65, 0x78, 0x65, 0x00, 0x3C, 0x00, 0x08, 0x00, 0x04, 0x00, 0xEF, 0xBE, 0xED, 0x3A, 0xC5 };

		//..:..*...JD.....
		//BAED3AC5BA2A0000004A440000000001

		BYTE bJDCheck[] = { 0xBA, 0xED, 0x3A, 0xC5, 0xBA, 0x2A, 0x00, 0x00, 0x00, 0x4A, 0x44, 0x00, 0x00, 0x00, 0x00, 0x01 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bcmdCheck, sizeof(bcmdCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bcmdCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bJDCheck, sizeof(bJDCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 5-03-2018
		//Customer Kishor Jadhav
		//s.i.i.n.u...e.x.e.......K...
		//7300690069006E0075002E006500780065000000180000004B000000

		BYTE bsiinuCheck[] = { 0x73, 0x00, 0x69, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x75, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x4B, 0x00, 0x00, 0x00 };

		//.u.n.i.i.s.\.s.i.i.n.u...e.x.e.../.
		//0075006E006900690073005C007300690069006E0075002E0065007800650002002F00

		BYTE buniisCheck[] = { 0x00, 0x75, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x69, 0x00, 0x73, 0x00, 0x5C, 0x00, 0x73, 0x00, 0x69, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x75, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x02, 0x00, 0x2F, 0x00 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bsiinuCheck, sizeof(bsiinuCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bsiinuCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], buniisCheck, sizeof(buniisCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;
		
		//Added By pradip on 09-05-2018
		//\.c.m.d...e.x.e.F./.c. 
		//5C0063006D0064002E0065007800650046002F006300

		BYTE bcmdfCheck[] = { 0x5C, 0x00, 0x63, 0x00, 0x6D, 0x00, 0x64, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x46, 0x00, 0x2F, 0x00, 0x63, 0x00 };

		//\.D.r.e.a.m.w.e.a.v.e.r._.M.X.\.d.r.e.a.m.w.e.a.v.e.r._.m.x...e.x.e
		//5C0044007200650061006D007700650061007600650072005F004D0058005C0064007200650061006D007700650061007600650072005F006D0078002E006500780065

		BYTE bdreamCheck[] = { 0x5C, 0x00, 0x44, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x77, 0x00, 0x65, 0x00, 0x61, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x5F, 0x00, 0x4D, 0x00, 0x58, 0x00, 0x5C, 0x00, 0x64, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x77, 0x00, 0x65, 0x00, 0x61, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x5F, 0x00, 0x6D, 0x00, 0x78, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bcmdfCheck, sizeof(bcmdfCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bcmdfCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bdreamCheck, sizeof(bdreamCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;
		
		//Added By pradip on 11-06-2018
		//t=!b .cmd.exe.<.
		//743D21622000636D642E657865003C00

		BYTE btbCheck[] = { 0x74, 0x3D, 0x21, 0x62, 0x20, 0x00, 0x63, 0x6D, 0x64, 0x2E, 0x65, 0x78, 0x65, 0x00, 0x3C, 0x00 };

			//..H.p.H.p*
			//BEA1481070A14810702A

		BYTE bhpCheck[] = { 0xBE, 0xA1, 0x48, 0x10, 0x70, 0xA1, 0x48, 0x10, 0x70, 0x2A };


			if (dwBytesRead < 0x20)
				goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], btbCheck, sizeof(btbCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(btbCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bhpCheck, sizeof(bhpCheck)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added By pradip on 11-09-2018
		///.c. .".{.7.1.f.0.f.9.9.0.-.2.4.b.a.
		//2F006300200022007B00370031006600300066003900390030002D003200340062006100

		BYTE bhiddenCheck[] = { 0x2F, 0x00, 0x63, 0x00, 0x20, 0x00, 0x22, 0x00, 0x7B, 0x00, 0x37, 0x00, 0x31, 0x00, 0x66, 0x00, 0x30, 0x00, 0x66, 0x00, 0x39, 0x00, 0x39, 0x00, 0x30, 0x00, 0x2D, 0x00, 0x32, 0x00, 0x34, 0x00, 0x62, 0x00, 0x61, 0x00 };

		//d.8.a.e.d.0.2.5.b.0.0.8...e.x.e
		//6400380061006500640030003200350062003000300038002E006500780065

		BYTE bhidden2Check[] = { 0x64, 0x00, 0x38, 0x00, 0x61, 0x00, 0x65, 0x00, 0x64, 0x00, 0x30, 0x00, 0x32, 0x00, 0x35, 0x00, 0x62, 0x00, 0x30, 0x00, 0x30, 0x00, 0x38, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };


		if (dwBytesRead < 0x20)
			goto Cleanup;


		bFound = bWScriptFound = false;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bhiddenCheck, sizeof(bhiddenCheck)) == 0x00)
				{
					bWScriptFound = true;
					dwIter += sizeof(bhiddenCheck);
				}
				continue;
			}
			if (bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bhidden2Check, sizeof(bhidden2Check)) == 0x00)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
			goto Cleanup;


		//Added by pradip on date 04-12-2018
		//c.t.t.j.w.g.a.u...e.x.e
		//6300740074006A0077006700610075002E006500780065

		BYTE bCttjwgauExeCheck[] = { 0x63, 0x00, 0x74, 0x00, 0x74, 0x00, 0x6A, 0x00, 0x77, 0x00, 0x67, 0x00, 0x61, 0x00, 0x75, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		SetFilePointer(hFile, 0x27E, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bCttjwgauExeCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bCttjwgauExeCheck, sizeof(bCttjwgauExeCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}



		//Added by pradip on date 15-11-2018

	
		//b.c.3.e.f.a.b.3.4.2.f.7.2.c.6.e.7.e.5.d.9.b.6.1.1.3.6.e.d.b.0.3...e.x.e
		//620063003300650066006100620033003400320066003700320063003600650037006500350064003900620036003100310033003600650064006200300033002E006500780065

		BYTE bExePath[] = { 0x62, 0x00, 0x63, 0x00, 0x33, 0x00, 0x65, 0x00, 0x66, 0x00, 0x61, 0x00, 0x62, 0x00, 0x33, 0x00, 0x34, 0x00,
			                    0x32, 0x00, 0x66, 0x00, 0x37, 0x00, 0x32, 0x00, 0x63, 0x00, 0x36, 0x00, 0x65, 0x00, 0x37, 0x00, 0x65, 0x00, 
								0x35, 0x00, 0x64, 0x00, 0x39, 0x00, 0x62, 0x00, 0x36, 0x00, 0x31, 0x00, 0x31, 0x00, 0x33, 0x00, 0x36, 0x00, 
								0x65, 0x00, 0x64, 0x00, 0x62, 0x00, 0x30, 0x00, 0x33, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		SetFilePointer(hFile, 0x5A0, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bExePath))
			goto Cleanup;

	    dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bExePath, sizeof(bExePath)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}
	
		


		//Added by pradip on date 15-11-2018

	
		//.I.M.G.-.R.C.S.H.-.2.5.3.0.1.8...e.x.e
		//0049004D0047002D0052004300530048002D003200350033003000310038002E006500780065

		BYTE bIMGRCGHCheck[] = { 0x00, 0x49, 0x00, 0x4D, 0x00, 0x47, 0x00, 0x2D, 0x00, 0x52, 0x00, 0x43, 0x00, 0x53, 0x00, 0x48, 0x00, 0x2D, 0x00, 0x32, 0x00, 0x35, 0x00, 0x33, 0x00, 0x30, 0x00, 0x31, 0x00, 0x38, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };


		SetFilePointer(hFile, 0x654, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bIMGRCGHCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bIMGRCGHCheck, sizeof(bIMGRCGHCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

	

		//Added by pradip on date 15-11-2018

		//h.t.t.p.:././.t.r.a.d.e.s.k.y...w.e.b.s.i.t.e./.t.r.a.c.e./.P.o.s.h.P.a.y.l.o.a.d...p.s.1
		//68007400740070003A002F002F007400720061006400650073006B0079002E0077006500620073006900740065002F00740072006100630065002F0050006F00730068005000610079006C006F00610064002E007000730031

		BYTE bPoshpayloadCheck[] = { 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F,
									0x00, 0x74, 0x00, 0x72, 0x00, 0x61, 0x00, 0x64, 0x00, 0x65, 0x00, 0x73, 0x00, 0x6B, 0x00, 0x79, 0x00, 
									0x2E, 0x00, 0x77, 0x00, 0x65, 0x00, 0x62, 0x00, 0x73, 0x00, 0x69, 0x00, 0x74, 0x00, 0x65, 0x00, 0x2F,
									0x00, 0x74, 0x00, 0x72, 0x00, 0x61, 0x00, 0x63, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x50, 0x00, 0x6F, 0x00, 0x73, 0x00, 0x68, 0x00, 0x50, 0x00, 0x61, 0x00, 0x79, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x61, 0x00, 0x64, 0x00, 0x2E, 0x00, 0x70, 0x00, 0x73, 0x00, 0x31 };

		SetFilePointer(hFile, 0x35C, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bPoshpayloadCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bPoshpayloadCheck, sizeof(bPoshpayloadCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

	

		//Added by pradip on date 15-11-2018

		//.h.t.t.p.:././.k.l.o.t.h.e.z...c.o.m./.w.p.-.a.d.m.i.n./.j.s./.j.y.j.l...p.s.1
		//68007400740070003A002F002F006B006C006F007400680065007A002E0063006F006D002F00770070002D00610064006D0069006E002F006A0073002F006A0079006A006C002E007000730031

		BYTE bJyjlCheck[] = { 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 0x6B, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x74, 0x00, 0x68, 0x00, 0x65, 0x00, 0x7A, 0x00, 0x2E, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x2F, 0x00, 0x77, 0x00, 0x70, 0x00, 0x2D, 0x00, 0x61, 0x00, 0x64, 0x00, 0x6D, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x2F, 0x00, 0x6A, 0x00, 0x73, 0x00, 0x2F, 0x00, 0x6A, 0x00, 0x79, 0x00, 0x6A, 0x00, 0x6C, 0x00, 0x2E, 0x00, 0x70, 0x00, 0x73, 0x00, 0x31 };


		SetFilePointer(hFile, 0x3AC, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bJyjlCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bJyjlCheck, sizeof(bJyjlCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

	
		//Added by pradip on date 15-11-2018

		//h.t.t.p.:././.h.e.c.f.o.r.e.x...i.n.f.o./.w.p.-.i.n.c.l.u.d.e.s./.u.p.d...e.x.e
		//68007400740070003A002F002F0068006500630066006F007200650078002E0069006E0066006F002F00770070002D0069006E0063006C0075006400650073002F007500700064002E006500780065

		BYTE bUPDCheck[] = { 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 
							0x68, 0x00, 0x65, 0x00, 0x63, 0x00, 0x66, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x65, 0x00, 0x78, 0x00, 0x2E,
							0x00, 0x69, 0x00, 0x6E, 0x00, 0x66, 0x00, 0x6F, 0x00, 0x2F, 0x00, 0x77, 0x00, 0x70, 0x00, 0x2D, 0x00,
							0x69, 0x00, 0x6E, 0x00, 0x63, 0x00, 0x6C, 0x00, 0x75, 0x00, 0x64, 0x00, 0x65, 0x00, 0x73, 0x00, 0x2F,
							0x00, 0x75, 0x00, 0x70, 0x00, 0x64, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		SetFilePointer(hFile, 0x3AC, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bUPDCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bUPDCheck, sizeof(bUPDCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}


		//Added by pradip on date 15-11-2018

		//h.t.t.p.:././.u.t.t.a.r.b.a.n.g.l.a.o.v.e.r.s.e.a.s.l.t.d...c.o.m./.w.p.-.a.d.m.i.n./.j.s./.j.i.h.i.l.l...p.s.1
		//68007400740070003A002F002F0075007400740061007200620061006E0067006C0061006F0076006500720073006500610073006C00740064002E0063006F006D002F00770070002D00610064006D0069006E002F006A0073002F006A006900680069006C006C002E007000730031

		BYTE bJihillCheck[] = { 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 0x75, 0x00,
								0x74, 0x00, 0x74, 0x00, 0x61, 0x00, 0x72, 0x00, 0x62, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x6C, 0x00,
								0x61, 0x00, 0x6F, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x65, 0x00, 0x61, 0x00, 0x73, 0x00, 
								0x6C, 0x00, 0x74, 0x00, 0x64, 0x00, 0x2E, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x2F, 0x00, 0x77, 0x00, 
								0x70, 0x00, 0x2D, 0x00, 0x61, 0x00, 0x64, 0x00, 0x6D, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x2F, 0x00, 0x6A, 0x00,
								0x73, 0x00, 0x2F, 0x00, 0x6A, 0x00, 0x69, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x2E, 0x00, 
								0x70, 0x00, 0x73, 0x00, 0x31 };

		SetFilePointer(hFile, 0x3AC, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bJihillCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bJihillCheck, sizeof(bJihillCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

		//Added by pradip on date 15-11-2018

		//h.t.t.p.:././.1.1.4...1.1.6...8...1.9.8./.c.h.e.c.k./.p.n.g./.m.s.x.s.l...e.x.e
		//68007400740070003A002F002F003100310034002E003100310036002E0038002E003100390038002F0063006800650063006B002F0070006E0067002F006D007300780073006C002E006500780065

		BYTE bMSXSLCheck[] = { 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 0x31, 0x00,
			0x31, 0x00, 0x34, 0x00, 0x2E, 0x00, 0x31, 0x00, 0x31, 0x00, 0x36, 0x00, 0x2E, 0x00, 0x38, 0x00, 0x2E, 0x00, 0x31, 
			0x00, 0x39, 0x00, 0x38, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x68, 0x00, 0x65, 0x00, 0x63, 0x00, 0x6B, 0x00, 0x2F, 0x00,
			0x70, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x2F, 0x00, 0x6D, 0x00, 0x73, 0x00, 0x78, 0x00, 0x73, 0x00, 0x6C, 0x00, 0x2E,
			0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		SetFilePointer(hFile, 0x500, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bMSXSLCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bMSXSLCheck, sizeof(bMSXSLCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

		if (bFound)
			goto Cleanup;

		//Added by pradip on date 15-11-2018

		//\.\.D.o.c.u.m.e.n.t.s.\.\.m.e.l.i.7...p.s.1
		//5C005C0044006F00630075006D0065006E00740073005C005C006D0065006C00690037002E007000730031

		BYTE bMELI7Check[] = { 0x5C, 0x00, 0x5C, 0x00, 0x44, 0x00, 0x6F, 0x00, 0x63, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x65, 0x00,
								0x6E, 0x00, 0x74, 0x00, 0x73, 0x00, 0x5C, 0x00, 0x5C, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x37,
								0x00, 0x2E, 0x00, 0x70, 0x00, 0x73, 0x00, 0x31 };

		SetFilePointer(hFile, 0x244, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bMELI7Check))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bMELI7Check, sizeof(bMELI7Check)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

	
		//Added by pradip on date 15-11-2018

		//'.g.r.4.'.+.'.5.w.q...'.+.'.p.s.'.+.'.1.
		//270067007200340027002B0027003500770071002E0027002B0027007000730027002B0027003100

		BYTE bGr45wqCheck[] = { 0x27, 0x00, 0x67, 0x00, 0x72, 0x00, 0x34, 0x00, 0x27, 0x00, 0x2B, 0x00, 0x27, 0x00, 0x35, 
									0x00, 0x77, 0x00, 0x71, 0x00, 0x2E, 0x00, 0x27, 0x00, 0x2B, 0x00, 0x27, 0x00, 0x70, 0x00, 0x73, 0x00, 0x27, 0x00,
									0x2B, 0x00, 0x27, 0x00, 0x31, 0x00 };

		SetFilePointer(hFile, 0x35C, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bGr45wqCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bGr45wqCheck, sizeof(bGr45wqCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

		//Added by pradip on date 26-11-2018

		//4.0.c.c.8.7.b.0.3.9.0.b...e.x.e
		//3400300063006300380037006200300033003900300062002E006500780065

		BYTE bExepathCheck[] = { 0x34, 0x00, 0x30, 0x00, 0x63, 0x00, 0x63, 0x00, 0x38,
			0x00, 0x37, 0x00, 0x62, 0x00, 0x30, 0x00, 0x33, 0x00, 0x39,
			0x00, 0x30, 0x00, 0x62, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };

		SetFilePointer(hFile, 0x1A2, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bExepathCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bExepathCheck, sizeof(bExepathCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

		//Added by pradip on date 26-11-2018
		//\.S.y.s.t.e.m.V.o.l.u.m.e.I.n.f.o.r.m.a.t.i.o.n...e.x.e
		//5C00530079007300740065006D0056006F006C0075006D00650049006E0066006F0072006D006100740069006F006E002E006500780065

		BYTE bVolumeInfoExeCheck[] = { 0x5C, 0x00, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6D, 0x00,
			0x56, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x49, 0x00,
			0x6E, 0x00, 0x66, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x74, 0x00,
			0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x2E, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65 };


		SetFilePointer(hFile, 0x160, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bVolumeInfoExeCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bVolumeInfoExeCheck, sizeof(bVolumeInfoExeCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

		//Added by pradip on date 26-11-2018

		//4.0.c.c.8.7.b.0.3.9.0.b...e.x.e
		//3400300063006300380037006200300033003900300062002E006500780065

		SetFilePointer(hFile, 0x1A2, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bExepathCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bExepathCheck, sizeof(bExepathCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}

		//Added by pradip on date 26-11-2018
		//\.S.y.s.t.e.m.V.o.l.u.m.e.I.n.f.o.r.m.a.t.i.o.n...e.x.e
		//5C00530079007300740065006D0056006F006C0075006D00650049006E0066006F0072006D006100740069006F006E002E006500780065

		SetFilePointer(hFile, 0x160, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bVolumeInfoExeCheck))
			goto Cleanup;

		dwIter = 0x00;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bWScriptFound)
			{
				if (_memicmp(&bFileData[dwIter], bVolumeInfoExeCheck, sizeof(bVolumeInfoExeCheck)) == 0x00)
				{
					bFound = bWScriptFound = true;
					goto Cleanup;
				}
			}
		}


		//.lnk files created by malware "Gamarue" family has less than 1800 bytes
		if (dwFileSize > 0x708)
			goto Cleanup;

		//Added by Vilas on 20 May 2015
		//Samples given by Santosh Gadkar
		//.lnk files created by malware "Gamarue" family and "NetWorm" detected as K7AntiVirus
		BYTE	bwNNDQ[24] = { 0x0B, 0x00, 0x00, 0xA0, 0x77, 0x4E, 0xC1, 0x1A, 0xE7, 0x02, 0x5D, 0x4E, 0xB7, 0x44, 0x2E, 0xB1,
			0xAE, 0x51, 0x98, 0xB7, 0xD5, 0x00, 0x00, 0x00 };

		BYTE	bSID[12] = { 0x53, 0x00, 0x2D, 0x00, 0x31, 0x00, 0x2D, 0x00, 0x35, 0x00, 0x2D, 0x00 };

		BYTE	bCheckData[19] = { 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0xA0, 0x58, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00 };

		DWORD	dwTemp = 0x00;
		bool	bThirdFound = false;
		bool	bFourthFound = false;

		dwBytesRead = 0x00;
		ZeroMemory(bFileData, sizeof(bFileData));
		ReadFile(hFile, bFileData, 0x400, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		dwCStartLocation = dwIter = 0x00;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!dwCStartLocation)
			{
				if (_memicmp(&bFileData[dwIter], bwNNDQ, sizeof(bwNNDQ)) == 0x00)
				{
					dwIter += sizeof(bwNNDQ);
					dwCStartLocation = dwIter;
				}

				continue;
			}

			//if ((dwCStartLocation) && (!bFourthFound) )
			if (dwCStartLocation)
			{
				if (!dwTemp)
				{
					if (_memicmp(&bFileData[dwIter], bSID, sizeof(bSID)) == 0x00)
					{

						dwIter += sizeof(bSID);
						bThirdFound = true;
						continue;
					}

				}

				if (bThirdFound)
				{
					if ((bFileData[dwIter] == 0x2D) && (bFileData[dwIter + 1] == 0x00))
						bFourthFound = true;
				}

				if (bFourthFound)
				{
					if (_memicmp(&bFileData[dwIter], bCheckData, sizeof(bCheckData)) == 0x00)
					{
						break;
					}
				}

			}

			/*	bFound = true;
			dwIter += sizeof(bCheckData);
			for (; dwTemp < dwCStartLocation; dwTemp++)
			{
			if (m_szComputerName[dwTemp] != bFileData[dwIter++])
			{
			bFound = false;
			break;
			}
			}

			break;
			*/
		}

		if ((bFourthFound) &&
			((*((DWORD *)&bFileData[dwBytesRead - 0x04]) == 0x00) && ((bFileData[dwBytesRead - 0x05] & 0xE0) == 0xE0))
			)
			bFound = true;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSplVirusScan::CheckForAutorunLNKFile", 0, 0, true, SECONDLEVEL);
		bFound = false;
	}

Cleanup:

	return bFound;
}

bool CWWizSplVirusScan::CheckForThumbsDBFile(LPBYTE	lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		BYTE	bMuslimah[0x08] = { 0x6D, 0x75, 0x73, 0x6C, 0x69, 0x6D, 0x61, 0x68 };

		if (memcmp(&lpbBuffer[0x05], bMuslimah, sizeof(bMuslimah)) != 0x00)
			goto Cleanup;

		BYTE	bYuyun[8] = { 0x6D, 0x65, 0x3A, 0x59, 0x75, 0x79, 0x75, 0x6E };

		if (memcmp(&lpbBuffer[0x3E], bYuyun, sizeof(bYuyun)) != 0x00)
			goto Cleanup;

		BYTE	bFileData[0x40] = { 0 };
		DWORD	dwBytesRead = 0x00;

		SetFilePointer(hFile, 0x46A, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x35, &dwBytesRead, NULL);

		BYTE bRunWScriptExe[0x35] = { 0x52, 0x75, 0x6E, 0x20, 0x22, 0x57, 0x53, 0x63,
			0x72, 0x69, 0x70, 0x74, 0x2E, 0x65, 0x78, 0x65,
			0x20, 0x2F, 0x2F, 0x65, 0x3A, 0x56, 0x42, 0x53,
			0x63, 0x72, 0x69, 0x70, 0x74, 0x20, 0x22, 0x2B,
			0x74, 0x6D, 0x70, 0x74, 0x2B, 0x22, 0x20, 0x22,
			0x22, 0x22, 0x2B, 0x51, 0x2B, 0x22, 0x22, 0x22,
			0x22, 0x0A, 0x0A, 0x00, 0x27
		};


		if (memcmp(bFileData, bRunWScriptExe, sizeof(bRunWScriptExe)) == 0x00)
			bFound = bVirusFound = true;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSplVirusScan::CheckForThumbsDBFile", 0, 0, true, SECONDLEVEL);
		bVirusFound = false;
	}

Cleanup:

	return bVirusFound;
}

bool CWWizSplVirusScan::CheckForCantix(LPBYTE lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;

		if (IsBadReadPtr(lpbBuffer, dwSize))
			goto Cleanup;

		//microsot
		//BYTE	microsot[0x08] = { 0x6D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66 };
		if ((*((DWORD *)&lpbBuffer[0x0C]) != 0x7263696D) || (*((DWORD *)&lpbBuffer[0x10]) != 0x666F736F))
			goto Cleanup;


		//redir.dll
		//BYTE	microsot[0x08] = { 0x72, 0x65, 0x64, 0x69, 0x72, 0x2E, 0x64, 0x6C, 0x6C };
		if ((*((DWORD *)&lpbBuffer[0x20]) != 0x69646572) || (*((DWORD *)&lpbBuffer[0x24]) != 0x6C642E72))
			goto Cleanup;

		//Windows.Serviks.
		BYTE	bWindowsServiks[0x10] = { 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x6B, 0x73, 0x0D };

		BYTE	bFileData[0x200] = { 0 };
		DWORD	dwBytesRead = 0x00, dwIter = 0x00;

		SetFilePointer(hFile, 0x90, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bWindowsServiks))
			goto Cleanup;

		dwBytesRead -= sizeof(bWindowsServiks);
		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bWindowsServiks, sizeof(bWindowsServiks)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		if (!bVirusFound)
			goto Cleanup;


		//Run "WScript.exe //e:VBScript "
		BYTE bRunWScript[0x1F] = { 0x52, 0x75, 0x6E, 0x20, 0x22, 0x57, 0x53, 0x63,
			0x72, 0x69, 0x70, 0x74, 0x2E, 0x65, 0x78, 0x65,
			0x20, 0x2F, 0x2F, 0x65, 0x3A, 0x56, 0x42, 0x53,
			0x63, 0x72, 0x69, 0x70, 0x74, 0x20, 0x22
		};

		bVirusFound = false;
		dwBytesRead = 0x00;
		ZeroMemory(bFileData, sizeof(bFileData));

		SetFilePointer(hFile, 0xF50, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);

		if (dwBytesRead < sizeof(bRunWScript))
			goto Cleanup;

		dwBytesRead -= sizeof(bRunWScript);
		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bRunWScript, sizeof(bRunWScript)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		bFound = bVirusFound;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSplVirusScan::CheckForCantix", 0, 0, true, SECONDLEVEL);
		bVirusFound = false;
	}
Cleanup:
	return bVirusFound;
}

bool CWWizSplVirusScan::CheckforDesktopVBS(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound)
{
	bool bVirusFound = false;

	try
	{
		bFound = bVirusFound;
		if (IsBadReadPtr(lpbBuffer, dwBufSize))
			goto Cleanup;

		unsigned char bStartData[0x10] = {
			0x6F, 0x6E, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x72, 0x65, 0x73, 0x75, 0x6D, 0x65, 0x20
		};

		if (memcmp(lpbBuffer, bStartData, sizeof(bStartData)) != 0x00)
		{
			goto Cleanup;
		}

		//Encrypted data in function call which used to	ExecuteGlobal
		unsigned char bParameterData[0x2C] = {
			0x2B, 0x49, 0x44, 0x41, 0x67, 0x64, 0x47, 0x68, 0x6C, 0x62, 0x67, 0x30, 0x4B, 0x61, 0x57, 0x59,
			0x67, 0x49, 0x47, 0x52, 0x79, 0x61, 0x58, 0x5A, 0x6C, 0x4C, 0x6D, 0x52, 0x79, 0x61, 0x58, 0x5A,
			0x6C, 0x64, 0x48, 0x6C, 0x77, 0x5A, 0x53, 0x41, 0x67, 0x50, 0x53, 0x41
		};


		BYTE	bFileData[0x200] = { 0 };
		DWORD	dwBytesRead = 0x00, dwIter = 0x00;

		//Checking First time Encrypted data in function call which used to	ExecuteGlobal 
		SetFilePointer(hFile, 0x800, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);
		if (dwBytesRead < sizeof(bParameterData))
		{
			goto Cleanup;
		}

		dwBytesRead -= sizeof(bParameterData);
		dwIter = 0x00;

		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bParameterData, sizeof(bParameterData)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		if (!bVirusFound)
		{
			goto Cleanup;
		}


		//Checking Second time Encrypted data in function call which used to	ExecuteGlobal
		bVirusFound = false;
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0xE00, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);
		if (dwBytesRead < sizeof(bParameterData))
		{
			goto Cleanup;
		}

		dwBytesRead -= sizeof(bParameterData);
		dwIter = 0x00;

		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bParameterData, sizeof(bParameterData)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		if (!bVirusFound)
		{
			goto Cleanup;
		}

		//Checking Third time Encrypted data in function call which used to	ExecuteGlobal
		bVirusFound = false;
		ZeroMemory(bFileData, sizeof(bFileData));
		SetFilePointer(hFile, 0x1900, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileData, 0x200, &dwBytesRead, NULL);
		if (dwBytesRead < sizeof(bParameterData))
		{
			goto Cleanup;
		}

		dwBytesRead -= sizeof(bParameterData);
		dwIter = 0x00;

		for (; dwIter<dwBytesRead; dwIter++)
		{
			if (memcmp(&bFileData[dwIter], bParameterData, sizeof(bParameterData)) == 0x00)
			{
				bVirusFound = true;
				break;
			}
		}

		bFound = bVirusFound;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSplVirusScan::CheckforDesktopVBS", 0, 0, true, SECONDLEVEL);
		bVirusFound = false;
	}
Cleanup:
	return bVirusFound;
}

/***************************************************************************************************
*  Function Name  : CheckForVBEVBSAtRemovalDrive
*  Description    : function to scan vbs/vbe file prsent at root of removable drive
*  Author Name    : Pradip
*  Date           : 1 August 2018
****************************************************************************************************/
bool CWWizSplVirusScan::CheckForVBEVBSAtRemovalDrive(LPCTSTR lpFileName, HANDLE hFile, bool &bFound, LPTSTR lpszVirusName)
{
	bool bVirusFound = false;

	try
	{
		
		//To check vbs and vbe file at AppData location
		//Added by pradip on 12-09-2018 
		if ((StrStrI(lpFileName, L"\\Users\\") ) ||
			(StrStrI(lpFileName, L"\\ProgramData\\"))
			)
		{
			//Added to avoid FP's for goood software
			//Added by Vilas on 26-09-2018 
			if ((!StrStrI(lpFileName, L"\\Programs\\")) &&
				(!StrStrI(lpFileName, L"\\Microsoft\\")) &&
				(!StrStrI(lpFileName, L"\\VMware\\")) &&
				(!StrStrI(lpFileName, L"\\MAILSPRING\\")) &&	//Added by Vilas on 16-10-2018 to avoid FP of VBE/VBS files present in VMware folder
				(!StrStrI(lpFileName, L"\\Desktop\\")))
			{
				if (StrStrI(lpFileName, L".vbe"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.VBE");
					goto Cleanup;
				}

				if (StrStrI(lpFileName, L".vbs"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.VBS");
					goto Cleanup;
				}

				//Added on date 24-9-2018 by pradip
				if (StrStrI(lpFileName, L".wsf"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.WSF");
					goto Cleanup;
				}

			//Commented on 18-12-2018 by Pradip to avoid FP
			/*	if (StrStrI(lpFileName, L".hta"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.html");
					goto Cleanup;
				}*/
			}

			//For .wsf file 
			if ((!StrStrI(lpFileName, L"\\OneDrive\\")) && (!StrStrI(lpFileName, L"\\Desktop\\")))
			{
				if (StrStrI(lpFileName, L".wsf"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.wsf");
					goto Cleanup;
				}
			}
			//Added by pradip on date 17-12-2018 
			//Check vbs,vbe,exe,dll,wsf and lnk file 
			if ((StrStrI(lpFileName, L"\\Windows\\Start Menu\\Programs\\Startup\\")))
			{
				if (StrStrI(lpFileName, L".vbs"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Suspicious.vbs");
					goto Cleanup;
				}
			}
			
			/*
			//Commented on 09-04-2019 by Vilas to avoid FP's at startup location for Good software
				if (StrStrI(lpFileName, L".lnk"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Suspicious.lnk");
					goto Cleanup;
				}
			*/
				if (StrStrI(lpFileName, L".vbe"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Suspicious.vbe");
					goto Cleanup;
				}


			/*
			//Commented on 09-04-2019 by Vilas to avoid FP's at startup location for Good software
				if (StrStrI(lpFileName, L".lnk"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Suspicious.lnk");
					goto Cleanup;
		}
			*/
				if (StrStrI(lpFileName, L".vbe"))
				{
					bFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Suspicious.vbe");
					goto Cleanup;
				}
		}
        
		const wchar_t *pFileName = wcsrchr(lpFileName, '\\');
		//To check file at root location
		if ((&lpFileName[2]) != (&pFileName[0]))
			     goto Cleanup;
		
		TCHAR szFileName[0x08] = { 0 };
		
		szFileName[0x00] = lpFileName[0x00];
		szFileName[0x01] = lpFileName[0x01];
		szFileName[0x02] = lpFileName[0x02];

		//Added for Autorun.inf at root location of any drive
		//if (wcsstr(pFileName, L"\\autorun.inf") || wcsstr(pFileName, L"\\AUTORUN.INF"))
		//Modified by Vilas on 04-09-2018 to support case-sensitive checking of file name
		if (StrStrI(pFileName, L"autorun.inf"))
		{
			bFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.inf");
			goto Cleanup;
		}
			
		//Added for .lnk at root location of any drive
		//Added by pradip on 11-09-2018 to support case-sensitive checking of file name
		if (StrStrI(pFileName, L".lnk"))
		{
			bFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.lnk");
			goto Cleanup;
		}

		//Added for .wsf at root location of any drive
		//Added by pradip on 24-09-2018 to support case-sensitive checking of file name
		if ((!StrStrI(lpFileName, L"\\Desktop\\")))
		{
			if (StrStrI(pFileName, L".wsf"))
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.wsf");
				goto Cleanup;
			}
		}

		//Added for .wsf at root location of any drive
		//Added by pradip on 24-09-2018 to support case-sensitive checking of file name
		if (StrStrI(pFileName, L".vbe"))
		{
			bFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.vbe");
			goto Cleanup;
		}

		//Added for .wsf at root location of any drive
		//Added by pradip on 24-09-2018 to support case-sensitive checking of file name
		if (StrStrI(pFileName, L".vbs"))
		{
			bFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.vbs");
			goto Cleanup;
		}

		UINT dwDriveType = GetDriveType(szFileName);

		if ((dwDriveType&DRIVE_REMOVABLE) == DRIVE_REMOVABLE)
		{
			//To check .vbe extention file
			//if ((wcsstr(pFileName, L".vbe")) || (wcsstr(pFileName, L".VBE")))
			//Modified to check the case-sensitive 
			if (StrStrI(pFileName, L".vbe"))
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.VBE");
				goto Cleanup;
			}

			//To check .vbs extention file
			//if ((wcsstr(pFileName, L".vbs")) || (wcsstr(pFileName, L".VBS")))
			//Modified to check the case-sensitive
			if (StrStrI(pFileName, L".vbs"))
			{
				bFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.MalScript.VBS");
				goto Cleanup;
			}
			
		}

	}
	catch (...)
	{
		bVirusFound = false;
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForVBEVBSAtRemovalDrive", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}

//Added on 06-02-2019 by Pradip to check the HTML maliciou files
bool CWWizSplVirusScan::CheckForMaliciousHtml(HANDLE hFile, DWORD dwSize, LPBYTE lpbBuffer, DWORD dwReadBytes, bool &bFound, LPTSTR lpszVirusName)
{
	bool bVirusFound = false;
	
	try
	{
	
		//Added by Pradip on date 14-3-2019 
		//To check dropped eml file on each directory (infection of Chir.B or Nimda)
		//696D697373796F75406274616D61696C2E6E65742E636E
		//imissyou@btamail.net.cn
		/*BYTE	bEMLHeader[0x17] = { 0x69, 0x6D, 0x69, 0x73, 0x73, 0x79, 0x6F, 0x75, 0x40, 0x62, 0x74, 0x61, 0x6D, 0x61,
			                         0x69, 0x6C, 0x2E, 0x6E, 0x65, 0x74, 0x2E, 0x63, 0x6E };

		if (_memicmp(lpbBuffer, bEMLHeader, sizeof(bEMLHeader)) != 0)
		{
			bFound = bVirusFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.Email-Worm.Win32.Runouce.b");
			goto Cleanup;
		}
		*/
		//<!DOCTYPE HTML
		//444F43545950452048544D4C
		BYTE	bHTMLHeader[0x0E] = {0x3C,0x21, 0x44,0x4F,0x43,0x54,0x59,0x50,0x45,0x20,0x48,0x54,0x4D,0x4C };
		//Commented By pradip on date 6-5-2019
		/*if (_memicmp(lpbBuffer, bHTMLHeader, sizeof(bHTMLHeader)) != 0)
		{
			bVirusFound = false;
			goto Cleanup;
		}*/


		BYTE	bBuffer[0x100] = { 0 };
		DWORD	dwBytesRead = 0x00;

		SetFilePointer(hFile, 0xAF, NULL, FILE_BEGIN);
		ReadFile(hFile, bBuffer, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < 0x24 )
			goto Cleanup;

		bFound = bVirusFound = false;
		dwReadBytes -= 0x24;
		for (DWORD i = 0x00; i < dwBytesRead; i++)
		{

			//7372633D	 22687474	703A2F2F
			if ((0x3D637273 == *((DWORD*)&bBuffer[i])) && (0x74746822 == *((DWORD*)&bBuffer[i + 0x04])) && (0x2F2F3A70 == *((DWORD*)&bBuffer[i + 0x08])))
			{

				//68796465	 72616261	64726174
				if ((0x65647968 == *((DWORD*)&bBuffer[i + 0x0C])) && (0x61626172 == *((DWORD*)&bBuffer[i + 0x10])) && (0x74617264 == *((DWORD*)&bBuffer[i + 0x14])))
				{

					//696E6773	2E636F6D	2F62726F
					if ((0x73676E69 == *((DWORD*)&bBuffer[i + 0x18])) && (0x6D6F632E == *((DWORD*)&bBuffer[i + 0x1C])) && (0x6F72622F == *((DWORD*)&bBuffer[i + 0x20])))
					{
						bFound = bVirusFound = true;
						wcscpy_s(lpszVirusName, 0xFE, L"WW.Trojan.HTML.Phishing.hyd");
						goto Cleanup;
					}

				}

			}
		}

		//Added by pradip on date 6-5-2019 for detection of html file given by Tarique
		dwBytesRead -= 0x10;
		for (DWORD i = 0x00; i < dwBytesRead; i++)
		{

			//4E555341 4B414D42 
			if ((0x4153554E == *((DWORD*)&bBuffer[i])) && (0x424D414B == *((DWORD*)&bBuffer[i + 0x04])))
			{

				//414E4741 4E22
				if ((0x41474E41 == *((DWORD*)&bBuffer[i + 0x08])) && (0x224E == *((WORD*)&bBuffer[i + 0x0C])))
				{

						bFound = bVirusFound = true;
						wcscpy_s(lpszVirusName, 0xFE, L"WW.Worm.Brontok.HTML.A");
						goto Cleanup;				
				}

			}
		}

	}


	catch (...)
	{
		bFound = bVirusFound = false;
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForMaliciousHtml", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}

//Added on 12-02-2019 by Pradip to check the JS malicious files
bool CWWizSplVirusScan::CheckForMaliciousJScript(HANDLE hFile, DWORD dwSize, bool &bVirusFound, LPTSTR lpszVirusName)
{

	try
	{

		DWORD	dwBytesRead = 0x00;
		BYTE   bFound = 0x00;
		bVirusFound = false;
		dwBytesRead = 0x00;
		BYTE bFileBuff[0x600] = { 0 };
		DWORD dwIter = 0x00;


		//Added By Pradip on date 15-4-2019
		SetFilePointer(hFile, 0x6E2EF, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileBuff, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		//\\gokuuuyt.exe
		BYTE bGokuuuytExe[] = { 0x5C,0x5C,0x67,0x6F,0x6B,0x75,0x75,0x75,0x79,0x74,0x2E,0x65,0x78,0x65 };
		
		dwIter = 0x00;
		dwBytesRead -= sizeof(bGokuuuytExe);

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (_memicmp(&bFileBuff[dwIter], bGokuuuytExe, sizeof(bGokuuuytExe)) == 0x00)
			{
				bVirusFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.Trojan.MSIL.Revenge.gen ");
				goto Cleanup;

			}
		}

		//Added By Pradip on date 28-3-2019
		ZeroMemory(bFileBuff, sizeof(bFileBuff));
		SetFilePointer(hFile, 0x150, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileBuff, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		////search.hdownloadmyinboxhelper.com
		BYTE bDownloadInbox[] = { 0x2F, 0x2F, 0x73, 0x65, 0x61, 0x72, 0x63, 0x68,
			0x2E, 0x68, 0x64, 0x6F, 0x77, 0x6E, 0x6C, 0x6F,
			0x61, 0x64, 0x6D, 0x79, 0x69, 0x6E, 0x62, 0x6F,
			0x78, 0x68, 0x65, 0x6C, 0x70, 0x65, 0x72, 0x2E, 0x63, 0x6F, 0x6D };

		dwIter = 0x00;
		dwBytesRead -= sizeof(bDownloadInbox);

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (_memicmp(&bFileBuff[dwIter], bDownloadInbox, sizeof(bDownloadInbox)) == 0x00)
			{
				bVirusFound = true;
				wcscpy_s(lpszVirusName, 0xFE, L"WW.Application.JS.Redirects.E ");
				goto Cleanup;

			}
		}

		//http: //search.searchtp.com?&ap=nocache&i_id=pac
		//http: //search.searchipdf.com?&ap=nocache&i_id=converter
		BYTE bHTTP[] = { 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F };

		BYTE bCOM[] = { 0x2E, 0x63, 0x6F, 0x6D };

		BYTE bSearch[] = { 0x73, 0x65, 0x61, 0x72, 0x63, 0x68, 0x2E };

		dwIter = 0x00;
		bFound = 0x00;
		dwBytesRead -= 0x07;

		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bFound)
			{
				if (_memicmp(&bFileBuff[dwIter], bHTTP, sizeof(bHTTP)) == 0x00)
				{
					bFound++;
					dwIter += sizeof(bHTTP);

				}
			}

			if (bFound == 0x01)
			{
				if (_memicmp(&bFileBuff[dwIter], bSearch, sizeof(bSearch)) == 0x00)
				{
					bFound++;
					dwIter += sizeof(bSearch);
					continue;
				}

			}

			if (bFound > 0x01)
			{
				if (_memicmp(&bFileBuff[dwIter], bCOM, sizeof(bCOM)) == 0x00)
				{
					bFound++;
					bVirusFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Application.JS.Redirects.E");
					goto Cleanup;
				}
			}
		}

	   //Added By pradip on date 18-3-2019
		ZeroMemory(bFileBuff, sizeof(bFileBuff));
		//SetFilePointer(hFile, 0x150, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileBuff, 0x100, &dwBytesRead, NULL);

		if (dwBytesRead < 0x20)
			goto Cleanup;

		//BYTE bHTTP[] = { 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F };

		//Modified  By pradip on 9-4-2019  to avoid FP
		// "http: //emailaccessonline.com
		BYTE bEmailAccessonline[] = { 0x65, 0x6D, 0x61, 0x69, 0x6C, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x6F, 0x6E, 0x6C, 0x69, 0x6E, 0x65 };

		//BYTE bEmail[] = { 0x65, 0x6D, 0x61, 0x69, 0x6C };

		//BYTE bCOM[] = { 0x2E, 0x63, 0x6F, 0x6D };

		bFound = 0x00;
		dwIter = 0x00;
		dwBytesRead -= 0x07;
		for (; dwIter < dwBytesRead; dwIter++)
		{
			if (!bFound)
			{
				if (_memicmp(&bFileBuff[dwIter], bHTTP, sizeof(bHTTP)) == 0x00)
				{
					bFound++;
					dwIter += sizeof(bHTTP);

				}
			}

			if (bFound == 0x01)
			{
				if (_memicmp(&bFileBuff[dwIter], bEmailAccessonline, sizeof(bEmailAccessonline)) == 0x00)
				{
					bFound++;
					dwIter += sizeof(bEmailAccessonline);
					continue;
				}

			}

			if (bFound > 0x01)
			{
				if (_memicmp(&bFileBuff[dwIter], bCOM, sizeof(bCOM)) == 0x00)
				{
					bFound++;
					bVirusFound = true;
					wcscpy_s(lpszVirusName, 0xFE, L"WW.Application.JS.Redirector.B ");
					goto Cleanup;
				}
			}
		}


		ZeroMemory(bFileBuff, sizeof(bFileBuff));
		SetFilePointer(hFile, 0x339, NULL, FILE_BEGIN);
		ReadFile(hFile, bFileBuff, 0x600, &dwBytesRead, NULL);

		if (dwBytesRead < 0x24)
			goto Cleanup;

		bVirusFound = false;
		dwIter = 0x00;
		bFound = 0x00;
		dwBytesRead -= 0x18;

		for (; dwIter < dwBytesRead; dwIter++)
		{

			if (!bFound)
			{
				//41006300 74006900 76006500
				if ((0x00630041 == *((DWORD*)&bFileBuff[dwIter])) && (0x00690074 == *((DWORD*)&bFileBuff[dwIter + 0x04])) &&
					(0x00650076 == *((DWORD*)&bFileBuff[dwIter + 0x08])))
				{

					//  58004F00 62006A00 65006300
					if ((0x004F0058 == *((DWORD*)&bFileBuff[dwIter + 0x0C])) && (0x006A0062 == *((DWORD*)&bFileBuff[dwIter + 0x10])) &&
						(0x00630065 == *((DWORD*)&bFileBuff[dwIter + 0x14])))
					{
						bFound = 0x01;
						dwIter += 0x1F;
						continue;
					}

				}

			}


			if (bFound == 0x01)
			{
				//002F0073 00790073 00740065
				if ((0x73002F00 == *((DWORD*)&bFileBuff[dwIter])) && (0x73007900 == *((DWORD*)&bFileBuff[dwIter + 0x04])) &&
					(0x65007400 == *((DWORD*)&bFileBuff[dwIter + 0x08])))
				{

					//006D0033 0032002F 0069002C
					if ((0x33006D00 == *((DWORD*)&bFileBuff[dwIter + 0x0C])) && (0x2F003200 == *((DWORD*)&bFileBuff[dwIter + 0x10])) &&
						(0x2C006900 == *((DWORD*)&bFileBuff[dwIter + 0x14])))
					{

						bFound++;
						break;
					}

				}

			}
		}

		if (bFound > 0x01)
		{
			bVirusFound = true;
			wcscpy_s(lpszVirusName, 0xFE, L"WW.Trojan.JS.Downloader.Crypter ");
			goto Cleanup;
		}
	
	}
	catch (...)
	{
		bVirusFound = false;
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::CheckForMaliciousJScript", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return bVirusFound;
}