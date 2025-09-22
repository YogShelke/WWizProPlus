/**********************************************************************************************************                	  Program Name          : WardWizCrashHandler.h
	  Description           : This class provides and functionality to create a crash and send the report to 
							  WardWiz website.
	  Author Name			: Ramkrushna Shelke                                                                            	  Date Of Creation      : 22 Jul 2014
	  Version No            : 1.0.0.5
	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper to create a crash dump.		22 Jul 2014
***********************************************************************************************************/
#pragma once
#include "stdafx.h"

typedef void (*CallBackFunctionPtr)(LPVOID lpParam);

class CWardWizCrashHandler
{
public:
	CWardWizCrashHandler(CString csAppName, CallBackFunctionPtr fnPtrCallBack);
	virtual ~CWardWizCrashHandler(void);
	bool Initialize();
	CString GetAppDir();
	CString GetModulePath(HMODULE hModule);
public:
	CallBackFunctionPtr m_fnPtrCallBack;
	CString				m_csAppName;
};
