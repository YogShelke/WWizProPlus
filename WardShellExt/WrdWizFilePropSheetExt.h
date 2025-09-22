/**********************************************************************************************************
Program Name          : FilePropSheetExt.cpp
Description           : The code sample demonstrates creating a Shell property sheet handler with C++.
						A property sheet extension is a COM object implemented as an in-proc server.
						The property sheet extension must implement the IShellExtInit and
						IShellPropSheetExt interfaces. A property sheet extension is instantiated
						when the user displays the property sheet for an object of a class for which
						the property sheet extension has been registered in the display specifier of
						the class. It enables you to add or replace pages. You can register and
						implement a property sheet handler for a file class, a mounted drive, a
						control panel application, and starting from Windows 7, you can install a
						property sheet handler to devices in Devices and Printers dialog.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 21 Nov 2017
Version No            : 2.6.0.69
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#pragma once

#include "stdafx.h"
#include "resource.h"
#include <windows.h>
#include <shlobj.h>     // For IShellExtInit and IShellPropSheetExt
#include "WrdWizSimpleExt.h"
#include "ISpyCommunicator.h"
#include "WardwizLangManager.h"

class CFilePropSheetExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFilePropSheetExt, &CLSID_FilePropSheetExt>,
	public IShellExtInit, 
	public IShellPropSheetExt
{
public:

	DECLARE_REGISTRY_RESOURCEID(IDR_DISPLAYCPLEXT)

	BEGIN_COM_MAP(CFilePropSheetExt)
		COM_INTERFACE_ENTRY(IShellPropSheetExt)
		COM_INTERFACE_ENTRY(IShellExtInit)
	END_COM_MAP()

    // IShellExtInit
    IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

    // IShellPropSheetExt
    IFACEMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
    IFACEMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam);

	CFilePropSheetExt();
    PCWSTR GetSelectedFile();
	bool StartScanUsingService(CString csFilePath);
	CString GetProdutVersion();
	CString GetDatabaseVersion();
	CString GetScanEngineVersion();
	CString GetShortFilePath(CString csFilePath);
	HBITMAP LoadBitmapResource(DWORD dwBitmapID);
	void MakeBold(HWND hwnd);
protected:
	~CFilePropSheetExt();

private:
    // Reference count of component.
    long					m_cRef;
    // The name of the selected file.
    wchar_t					m_szSelectedFile[MAX_PATH];
    // Property sheet page title.
    PCWSTR					pszPageTitle;
public:
	CString					m_csProdRegKey;
	DWORD					m_dwProductID;
	CString					m_csProdVersion;
	CString					m_csDBVersion;
	CString					m_csScanEngineVersion;
	CString					m_csProductEdition;
	CWardwizLangManager		m_objwardwizLangManager;
};