// SimpleShlExt.h : Declaration of the CSimpleShlExt

#ifndef __SIMPLESHLEXT_H_
#define __SIMPLESHLEXT_H_
#include "WardwizLangManager.h"
/////////////////////////////////////////////////////////////////////////////
// CSimpleShlExt

/***************************************************************************
Function Name  : OS_TYPE
Description    : enum of os version
Author Name    : Neha gharge
Date           : 22th April 2015
****************************************************************************/

enum __OSTYPE
{
	WINOSUNKNOWN_OR_NEWEST = 0,
	WINOS_95,
	WINOS_98,
	WINOS_2000,
	WINOS_NT,
	WINOS_XP,
	WINOS_VISTA,
	WINOS_WIN7,
	WINOS_WIN8,
	WINOS_WIN8_1,
	WINOS_XP64
};

//Neha Gharge Lnk get failed in getcommandstrings.23/4/2015
#define szHelpTextA "selected file(s) or folder(s) with WardWiz"
#define szHelpTextW L"selected file(s) or folder(s) with WardWiz"


class ATL_NO_VTABLE CSimpleShlExt : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSimpleShlExt, &CLSID_SimpleShlExt>,
    public IShellExtInit,
    public IContextMenu
{
public:
   CSimpleShlExt();

    DECLARE_REGISTRY_RESOURCEID(IDR_SIMPLESHLEXT)

    BEGIN_COM_MAP(CSimpleShlExt)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

public:
    // IShellExtInit
    STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);
	STDMETHODIMP InitializeSEH(LPCITEMIDLIST, LPDATAOBJECT, HKEY);
    // IContextMenu
    STDMETHODIMP GetCommandString(UINT_PTR, UINT, UINT*, LPSTR, UINT);
	STDMETHODIMP GetCommandStringSEH(UINT_PTR, UINT, UINT*, LPSTR, UINT);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
	STDMETHODIMP InvokeCommandSEH(LPCMINVOKECOMMANDINFO);
    STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);
	STDMETHODIMP QueryContextMenuSEH(HMENU, UINT, UINT, UINT, UINT);
	bool GetWardWizPathFromRegistry(TCHAR *szModuleName);
	bool GetWardWizPathFromRegistrySEH(TCHAR *szModuleName);
	DWORD DetectClientOSVersionSEH();
	DWORD DetectClientOSVersion();
	UINT m_iTotalFolderSelected;
	std::vector<std::wstring> m_vszFolderPaths;
	std::vector<std::wstring> m_vszFolderPathArguments;
	DWORD m_dwlenOfArguments;
	DWORD IsFileEncryptedSEH();
	DWORD IsFileEncrypted();
	void GetEnviormentVariablesForAllMachine();
	bool IsPathBelongsToOSReservDirectory(wstring csFilefolderPath);
	bool IsNetworkPathSelected(wstring wsPath);
	bool IsFileIsProtected(wstring wsPath);
	void CheckForOSOrNetworkPath();

	DWORD		m_dwWardWizOpt;
protected:
    TCHAR m_szFile [MAX_PATH];
	HBITMAP     m_hRegBmp;

public:

	TCHAR		 m_szWindow[MAX_PATH];
	TCHAR		 m_szProgramFile[MAX_PATH];
	TCHAR		 m_szProgramFilex86[MAX_PATH];
	TCHAR		 m_szAppData4[MAX_PATH];
	TCHAR		 m_szDriveName[MAX_PATH];
	TCHAR		 m_szArchitecture[MAX_PATH];
	TCHAR		 m_szBrowseFilePath[MAX_PATH];
	TCHAR		 m_szUserProfile[MAX_PATH];
	TCHAR		 m_szAppData[MAX_PATH];
	TCHAR		 m_szProgramData[MAX_PATH];
	TCHAR		 m_szModulePath[MAX_PATH];

	bool		 m_bIsSystemPath;
	bool		 m_bIsNetworkPath;
	DWORD		 m_dwProductID;
	CString		 m_csProdRegKey;
	CWardwizLangManager m_objwardwizLangManager;
};

#endif //__SIMPLESHLEXT_H_
