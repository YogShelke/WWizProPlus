/**********************************************************************************************************
Program Name			: BDClient.h
Description				: this file contains implementation of Bitdefender scan engine with below functionalities
*							1) Scan
*							2) Repair
*							3) Smart Scan
Author Name				: Ramkrushna Shelke
Date Of Creation		: 28 Aug 2019
Version No				: 3.6.0.3
Special Logic Used		:

Modification Log      :
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include "scanapi.h"
#include "scanerrors.h"
#include "smartdb.h"
#include "iTINRegWrapper.h"

# define _TAV(s)						L##s 

typedef int(*fnThreatScanner_InitializeEx)(const CHAR_T *, const CHAR_T *, CHAR_T *, InitializeParams *);
typedef int(*fnThreatScanner_CreateInstance)(CThreatScanner **scanner);
typedef int(*fnThreatScanner_SetIntOption)(CThreatScanner * scanner, int option, int value);
typedef int(*fnThreatScanner_SetStringOption)(CThreatScanner * scanner, int option, const CHAR_T * value);
typedef int(*fnThreatScanner_SetScanCallback2)(CThreatScanner * scanner, SCAN2_CALLBACK pfnCallback2, void * pContext);
typedef int(*fnThreatScanner_SetEnginesUnloadCallback)(ENGINES_UNLOAD_CALLBACK enginesUnloadCbk, void *pContext);
typedef int(*fnThreatScanner_ScanObjectByHandle)(CThreatScanner * scanner, void * hObject, const CHAR_T * szObjectName, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int accessorPID);
typedef int(*fnThreatScanner_InitializeMemoryScan)(CThreatScanner * scanner, const CHAR_T * mappingName, unsigned long mappingSize);
typedef int(*fnThreatScanner_UninitializeMemoryScan)(CThreatScanner * scanner);
typedef int(*fnThreatScanner_ScanMemoryEx)(CThreatScanner * scanner, const CHAR_T * objectName, int nObjectType, unsigned long * pdwObjectSize, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo);
typedef int(*fnThreatScanner_ScanPath)(CThreatScanner * scanner, int objectType, const CHAR_T * szPath, int accessorPID);
typedef int(*fnThreatScanner_ScanObject)(CThreatScanner * scanner, int objectType, const CHAR_T * szObjectPath, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int accessorPID, const CHAR_T * szObjectName);
typedef int(*fnThreatScanner_SetPasswordCallback)(CThreatScanner * scanner, PASSWORD_CALLBACK passwordCallback, void * context);
typedef int(*fnThreatScanner_SetExtCallback)(CThreatScanner * scanner, EXTSCAN_CALLBACK pfnExtCallback, void * pContext);
typedef int(*fnThreatScanner_GetOption)(CThreatScanner * scanner, int option, void * value);
typedef int(*fnThreatScanner_GetScanStatistics)(CThreatScanner * scanner, ScanStatistics **ppStatistics);
typedef int(*fnThreatScanner_DestroyInstance)(CThreatScanner * scanner);
typedef int(*fnThreatScanner_GetSmartDBFunctions)(SmartDBExportedFunctions * pFunctions);
typedef int(*fnThreatScanner_GetKeys)(CThreatScanner * scanner, void * pKeys, int nrFiles, void * pReserved);
typedef int(*fnThreatScanner_Uninitialize)();

typedef enum _GZ_SCAN_RESULT
{
	SCAN_STATE_ERROR = 1,
	SCAN_STATE_UNKNOWN,
	SCAN_STATE_SCANNED,
	SCAN_STATE_INFECTED,

	SCAN_STATE_MAX = 10
} GZ_SCAN_RESULT;

class CBDClient
{
public:
	CBDClient();
	virtual ~CBDClient();

	bool BD_initialize();
	bool BD_ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID);
	bool BD_uninitialize();
	bool BD_RepairFile(LPCTSTR szFilePath, DWORD dwSpyID);
	bool LoadRequiredModules();
	BOOL ScanFileInMemory(CThreatScanner * scanner, LPCTSTR filePath, LPTSTR lpVirusName);
	//Callbacks
	static void	 OnObjectScanComplete2(ScanCbkParams *params, int *pnScanAction, void *context);
	bool SetHeuristicSetting(int iHeuFlag);
	bool ReadHeuristicScanStatus();
private:
	HMODULE											m_pModule;
	fnThreatScanner_InitializeEx					m_pfnThreatScanner_InitializeEx;
	fnThreatScanner_CreateInstance					m_pfnThreatScanner_CreateInstance;
	fnThreatScanner_SetIntOption					m_pfnThreatScanner_SetIntOption;
	fnThreatScanner_SetStringOption					m_pfnThreatScanner_SetStringOption;
	fnThreatScanner_SetScanCallback2				m_pfnThreatScanner_SetScanCallback2;
	fnThreatScanner_SetEnginesUnloadCallback		m_pfnThreatScanner_SetEnginesUnloadCallback;
	fnThreatScanner_ScanPath						m_pfnThreatScanner_ScanPath;
	fnThreatScanner_SetPasswordCallback				m_pfnThreatScanner_SetPasswordCallback;
	fnThreatScanner_SetExtCallback					m_pfnThreatScanner_SetExtCallback;
	fnThreatScanner_GetOption						m_pfnThreatScanner_GetOption;
	fnThreatScanner_GetScanStatistics				m_pfnThreatScanner_GetScanStatistics;
	fnThreatScanner_ScanObject						m_pfnThreatScanner_ScanObject;
	fnThreatScanner_DestroyInstance					m_pfnThreatScanner_DestroyInstance;
	fnThreatScanner_Uninitialize					m_pfnThreatScanner_Uninitialize;
	fnThreatScanner_ScanMemoryEx					m_pfnThreatScanner_ScanMemoryEx;
	fnThreatScanner_InitializeMemoryScan			m_pfnThreatScanner_InitializeMemoryScan;
	fnThreatScanner_UninitializeMemoryScan			m_pfnThreatScanner_UninitializeMemoryScan;
	fnThreatScanner_ScanObjectByHandle				m_pfnThreatScanner_ScanObjectByHandle;
	fnThreatScanner_GetSmartDBFunctions				m_pfnThreatScanner_GetSmartDBFunctions;
	fnThreatScanner_GetKeys							m_pfnThreatScanner_GetKeys;
	SmartDBExportedFunctions						m_pSmartDBApi;
	CThreatScanner *								m_pscanner;
	bool											m_bIsHeuScan;
	CITinRegWrapper									m_objReg;
};

