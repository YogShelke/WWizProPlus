/**********************************************************************************************************
Program Name          : WWizSplVirusScan.h
Description           : Class to scan special type of viruses
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 14th Mar 2016
Version No            :
Special Logic Used    : This class is created for the scanning of special type of viruses

Modification Log      :
1. Ram Shelke	          : Created Class CWWizSplVirusScan       14 - 03 - 2016
***********************************************************************************************************/
#pragma once
class CWWizSplVirusScan
{
public:
	CWWizSplVirusScan();
	virtual ~CWWizSplVirusScan();
public:
	DWORD ScanFile(LPCTSTR lpFileName, LPTSTR lpVirusName, DWORD &dwSpyID, bool bRescan);
	//Modified by pradip on date 30-8-2018
	bool CheckForAutorunInfFile(LPBYTE	lpbBuffer, DWORD dwSize,HANDLE hFile, bool &bFound);
	bool CheckForAutorunLNKFile(LPCTSTR lpFileName, LPBYTE lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound, LPTSTR lpszVirusName);
	bool CheckForThumbsDBFile(LPBYTE	lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);
	bool CheckForCantix(LPBYTE lpbBuffer, DWORD dwSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);
	bool CheckforDesktopVBS(LPBYTE	lpbBuffer, DWORD dwBufSize, HANDLE hFile, DWORD dwFileSize, bool &bFound);
	
	//Added by pradip on date 1-08-2018 
	//To check VBE and VBS file present at the root of removable drive
	bool CheckForVBEVBSAtRemovalDrive(LPCTSTR lpFileName, HANDLE hFile, bool &bFound, LPTSTR lpszVirusName);
	//Added by pradip on date 29-11-2018 
	//bool CheckForBatchFile(LPCTSTR lpFileName, HANDLE hFile, bool &bFound, LPTSTR lpszVirusName);
	//bool CheckForVbsFile(LPCTSTR lpFileName, HANDLE hFile, bool &bFound, LPTSTR lpszVirusName);
	bool CheckForMaliciousHtml(HANDLE hFile, DWORD dwSize, LPBYTE lpbBuffer, DWORD dwReadBytes, bool &bFound, LPTSTR lpszVirusName);
	
	bool CheckForMaliciousJScript(HANDLE hFile, DWORD dwSize, bool &bFound, LPTSTR lpszVirusName);

	bool CheckForMaliciousXML(HANDLE hFile, DWORD dwSize, LPBYTE lpbBuffer, bool &bFound, LPTSTR lpszVirusName);


};

