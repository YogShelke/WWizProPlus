/**********************************************************************************************************
Program Name          : WardWizParseIni.h
Description           : Parse ini files for wardwiz update.
Author Name			  : Amol Jaware
Date Of Creation      : 20 Jan 2019
Version No            : 4.1.0.1
***********************************************************************************************************/

#pragma once

#ifndef _WARDWIZPARSEINI_H_
#define _WARDWIZPARSEINI_H_


#include <Windows.h>
#include <stdio.h>
#include<shlwapi.h>

#include "WardWizALUStruct.h"
#include "iTINRegWrapper.h"

class WardWizParseIniFile
{
	public:
		WardWizParseIniFile();
		~WardWizParseIniFile();

		DWORD GetLastErrorForParseDownloadedPatch_Ini();
		bool ParseDownloadedPatch_Ini(LPTSTR lpszIniPath, DWORD dwProductID, bool bProductUpdate, bool &bRetIsAnyProductChanges, bool bIsWow64);
		bool ParseDownloadedPatch_Ini4UpdtMgr(LPTSTR lpszIniPath, DWORD dwProductID, bool bProductUpdate, bool &bRetIsAnyProductChanges);
		bool ParseSectionAndCheckLatestFiles( DWORD dwCount, LPTSTR lpszSecName, LPTSTR lpszIniPath, LPTSTR lpszServerLocation );
		bool ParseSectionAndCheckLatestFiles4UpdtMgr(DWORD dwCount, LPTSTR lpszSecName, LPTSTR lpszIniPath, LPTSTR lpszServerLocation);
		bool ParseIniLine(LPTSTR lpszIniLine, LPTSTR lpszServerLocation );
		bool ParseIniLine4UpdtMgr(LPTSTR lpszIniLine, LPTSTR lpszServerLocation);
		bool CheckFilePresentIsLatest( PALUFILEINFO pALUFileInfo);
		bool IsCheckFileIsPresentLatestZipFile(PALUZIPFILEINFO pALUZipFileInfo);
		bool GetFilePath(LPTSTR lpszFilePath, LPTSTR lpszShortPath, LPTSTR lpszFileName );
		bool GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash);
		bool GetProductID( DWORD &dwProductID);
		bool GetDWORDValueFromRegistry(HKEY hMain, LPTSTR lpszSubKey, LPTSTR lpszValuneName, DWORD &dwProductID);
		bool GetScanLevelRegistry();

	protected:
		DWORD					m_dwLastError;

	public:
		DWORD					m_dwProductID;
		SCANLEVEL				m_eScanLevel;
		CITinRegWrapper			m_objReg;
};

#endif