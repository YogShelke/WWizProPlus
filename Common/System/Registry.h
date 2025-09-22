#pragma once
#include <atlbase.h>

class CRegistry
{
public:
	CRegistry(void);
	virtual ~CRegistry(void);
public:
	CString GetStringValue( HKEY hKeyArg, LPCTSTR keyNameArg, LPCTSTR valNameArg );
	bool SetStringValue(HKEY hKey, LPCTSTR keyNameArg, LPCTSTR valNameArg, LPCTSTR szData);
};
