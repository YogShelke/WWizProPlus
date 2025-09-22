// WrdwizSimpleExt.cpp : Implementation of DLL Exports.

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f SimpleExtps.mk in the project directory.
// WrdwizSimpleExt.cpp : implementation file
/**************************************************************************************************
*  Program Name: WrdwizSimpleExt.cpp                                                                                                    
*  Description: It is app class and it adds our option to the list of approved shell extensions. 
*  Author Name: Neha Gharge                                                                                                      
*  Date Of Creation: 2 Sept 2014
*  Version No: 1.9.0.0
**************************************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "WrdWizSimpleExt.h"
#include "WrdWizSimpleExt_i.c"
#include "WrdWizSimpleShlExt.h"
#include "WrdWizFilePropSheetExt.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SimpleShlExt, CSimpleShlExt)
OBJECT_ENTRY(CLSID_FilePropSheetExt, CFilePropSheetExt)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

/**********************************************************************************************************                     
*  Function Name  :	DllMain                                                     
*  Description    :	Dll entry point 
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_001
**********************************************************************************************************/
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	try
	{
		if (dwReason == DLL_PROCESS_ATTACH)
		{
			_Module.Init(ObjectMap, hInstance, &LIBID_SIMPLEEXTLib);
			DisableThreadLibraryCalls(hInstance);
		}
		else if (dwReason == DLL_PROCESS_DETACH)
			_Module.Term();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumShellExt::DllMain", 0, 0, true, SECONDLEVEL);
	}
    return TRUE;    // ok
}

/**********************************************************************************************************                     
*  Function Name  :	DllCanUnloadNow                                                     
*  Description    :	Used to determine whether the DLL can be unloaded by OLE 
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_002
**********************************************************************************************************/
STDAPI DllCanUnloadNow()
{
	try
	{
		return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumShellExt::DllCanUnloadNow", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	DllGetClassObject                                                     
*  Description    :	Returns a class factory to create an object of the requested type
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_003
**********************************************************************************************************/
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	try
	{
		return _Module.GetClassObject(rclsid, riid, ppv);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumShellExt::DllGetClassObject", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	DllRegisterServer                                                     
*  Description    :	Adds entries to the system registry
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_004
**********************************************************************************************************/
STDAPI DllRegisterServer()
{
    // If we're on NT, add ourselves to the list of approved shell extensions.

    // Note that you should *NEVER* use the overload of CRegKey::SetValue with
    // 4 parameters.  It lets you set a value in one call, without having to 
    // call CRegKey::Open() first.  However, that version of SetValue() has a
    // bug in that it requests KEY_ALL_ACCESS to the key.  That will fail if the
    // user is not an administrator.  (The code should request KEY_WRITE, which
    // is all that's necessary.)

	try
	{
		CRegKey reg;
		LONG    lRet;
		bool    bNT = (0 == (GetVersion() & 0x80000000));

		if (0 == (GetVersion() & 0x80000000UL))
		{



			lRet = reg.Open(HKEY_LOCAL_MACHINE,
				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
				KEY_SET_VALUE);

			if (ERROR_SUCCESS != lRet)
				return E_ACCESSDENIED;

			// lRet = reg.SetValue (_T("SimpleShlExt extension"), 
			//                     _T("{5E2121EE-0300-11D4-8D3B-444553540000}") );
			lRet = reg.SetValue(_T("WrdWizShlExt extension"),
				_T("{5E2121EE-0300-11D4-8D3B-444553540000}"));

		
			reg.Close();
			
			if (ERROR_SUCCESS != lRet)
				return E_ACCESSDENIED;
		}

		// Add a key to have our extension loaded by the Display applet.  Unfortunately
		// the key is different between 9x and 2000, so it has to be done in code.
		// I haven't tested this on NT 4 yet (don't have a system handy) so I don't
		// know the right key to use on NT 4.  For now I'll use the same key as on 2K.
		TCHAR szKey[512] = { 0 };
		wsprintf(szKey, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\%s\\shellex\\PropertySheetHandlers\\WardWizPage"),
			bNT ? _T("Desk") : _T("Display"));

		lRet = reg.Create(HKEY_LOCAL_MACHINE, szKey, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE);

		if (ERROR_SUCCESS != lRet)
			return HRESULT_FROM_WIN32(lRet);

		lRet = reg.SetValue(_T("{5C8FA94F-F274-49B9-A5E5-D34C093F7846}"));

		if (ERROR_SUCCESS != lRet)
			return HRESULT_FROM_WIN32(lRet);


		// registers object, typelib and all interfaces in typelib
		return _Module.RegisterServer(FALSE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumShellExt::DllRegisterServer", 0, 0, true, SECONDLEVEL);
	}
	return S_OK;
}

/**********************************************************************************************************                     
*  Function Name  :	DllUnregisterServer                                                     
*  Description    :	Removes entries from the system registry
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_005
**********************************************************************************************************/
STDAPI DllUnregisterServer()
{
    // If we're on NT, remove ourselves from the list of approved shell extensions.
    // Note that if we get an error along the way, I don't bail out since I want
    // to do the normal ATL unregistration stuff too.
	try
	{
		bool    bNT = (0 == (GetVersion() & 0x80000000));
		if (0 == (GetVersion() & 0x80000000UL))
		{
			CRegKey reg;
			LONG    lRet;

			lRet = reg.Open(HKEY_LOCAL_MACHINE,
				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
				KEY_SET_VALUE);

			if (ERROR_SUCCESS == lRet)
			{
				// lRet = reg.DeleteValue ( _T("{5E2121EE-0300-11D4-8D3B-444553540000}") );
				lRet = reg.DeleteValue(_T("{5E2121EE-0300-11D4-8D3B-444553540000}"));
				reg.Close();
			}


			// Delete our entry under the Display/Desk applet reg key.
			TCHAR szKey[512];
			wsprintf(szKey, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\%s\\shellex\\PropertySheetHandlers\\WardWizPage"),
				bNT ? _T("Desk") : _T("Display"));
			RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);
		}

		return _Module.UnregisterServer(FALSE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumShellExt::DllUnregisterServer", 0, 0, true, SECONDLEVEL);
	}
}
