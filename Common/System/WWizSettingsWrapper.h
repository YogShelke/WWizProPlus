/**********************************************************************************************************
Program Name          : WWizSettingsWrapper.cpp
Description           : This class is for get user defined settings which are related to product functionality.
						This class read from PRODUCTSETTINGS.INI file and provides the value wherever it is
						required.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 31st May 2016
Version No            : 2.0.0.14
Modification Log      :
***********************************************************************************************************/
#pragma once
#include "stdafx.h"

class CWWizSettingsWrapper
{
public:
	CWWizSettingsWrapper();
	virtual ~CWWizSettingsWrapper();
public:
	static CString GetProductRegistryKey();
	static CString GetRegisteredCountry();
	static CString GetRegisteredState();
	static CString GetRegisteredCity();
	static CString GetRegisteredPinCode();
	static CString GetCurrentUserName();
	static CString GetCurrentOSName();
	static CString GetCurrentRamSize();
	static CString GetCurrentHardDiskSize();
	static CString GetCurrentProcessorSize();
	static CString GetCurrentComputerName();
	static CString GetDealerCode();
	static CString GetReferenceID();
	static CString GetEngineerName();
	static CString GetEngineerMobNo();
private:
	static bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
	static bool GetCOMDLLModulePath(LPTSTR lpszPath, DWORD dwSize);
};

