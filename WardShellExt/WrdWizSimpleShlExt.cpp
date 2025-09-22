// WrdWizSimpleShlExt.cpp : Implementation of CSimpleShlExt

// WrdWizSimpleShlExt.cpp : implementation file
/****************************************************
*  Program Name: WrdWizSimpleShlExt.cpp                                                                                                    
*  Description: 
*  Author Name: Neha Gharge                                                                                                      
*  Date Of Creation: 2 Sept 2014
		Changes:	Removed Warnings for : _tcscpy, _tcscat
											added GetproductID functionality from common
*  Version No: 1.0.0.9
****************************************************/

#include "stdafx.h"
#include "resource.h"
#include "WrdWizSimpleExt.h"
#include "WrdWizSimpleShlExt.h"

#include <string.h>
#include <Sfc.h>
//#include "afxdlgs.h"

/////////////////////////////////////////////////////////////////////////////
// CSimpleShlExt

/**********************************************************************************************************
*  Function Name  :	Initialize
*  Description    :	Initialize variables.Get a data from explorer like file which are selected.
*  Author Name    : Neha Gharge
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_006
**********************************************************************************************************/

STDMETHODIMP CSimpleShlExt::Initialize(
	LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
	__try
	{
		return InitializeSEH(pidlFolder, pDataObj, hProgID);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()", 0, 0, true, SECONDLEVEL);
		return E_INVALIDARG;
	}
	return E_INVALIDARG;
}

/**********************************************************************************************************
*  Function Name  :	Initialize
*  Description    :	Initialize  member varialble.
*  Author Name    : Lalit kumawat
*  Date           : 7-23-2015
*  SR_NO		  : 
**********************************************************************************************************/
 CSimpleShlExt::CSimpleShlExt()
{

		memset(&m_szWindow, 0, MAX_PATH);
		memset(&m_szProgramFile, 0, MAX_PATH);
		memset(&m_szProgramFilex86, 0, MAX_PATH);
		memset(&m_szAppData4, 0, MAX_PATH);
		memset(&m_szDriveName, 0, MAX_PATH);
		memset(&m_szArchitecture, 0, MAX_PATH);
		memset(&m_szBrowseFilePath, 0, MAX_PATH);
		memset(&m_szAppData, 0, MAX_PATH);
		memset(&m_szProgramData, 0, MAX_PATH);
		memset(&m_szModulePath, 0, MAX_PATH);
		m_bIsNetworkPath = false;
		
}

/**********************************************************************************************************                     
*  Function Name  :	Initialize                                                     
*  Description    :	Initialize variables.Get a data from explorer like file which are selected.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_006
**********************************************************************************************************/

STDMETHODIMP CSimpleShlExt::InitializeSEH (
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID )
{
	try
	{
		FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg = { TYMED_HGLOBAL };
		HDROP     hDrop;
		m_iTotalFolderSelected = 0;
		m_dwlenOfArguments = 0;
		
		if (pDataObj == NULL)
			return E_FAIL;

		// Look for CF_HDROP data in the data object.

		if (FAILED(pDataObj->GetData(&fmt, &stg)))
		{
			// Nope! Return an "invalid argument" error back to Explorer.
			return E_INVALIDARG;
		}


		// Get a pointer to the actual data.
		hDrop = (HDROP)GlobalLock(stg.hGlobal);
		
		// Make sure it worked.
		if (NULL == hDrop)
			return E_INVALIDARG;

		// Sanity check - make sure there is at least one filename.
		UINT uNumFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		m_iTotalFolderSelected = uNumFiles;

		HRESULT hr = S_OK;

		if (0 == uNumFiles)
		{
			GlobalUnlock(stg.hGlobal);
			ReleaseStgMedium(&stg);
			return E_INVALIDARG;
		}

		//Get the file name for total no of files selected 
		// Get the name of the first file and store it in our member variable m_szFile.
		m_vszFolderPaths.clear();

		//Ram: Get here a Product ID to check for product version
		//CWardwizLangManager objLangManager;
		m_csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
		m_dwProductID = m_objwardwizLangManager.GetSelectedProductID();

		//Issue no : 700 If user select more than 6 files both option is not coming
		//Neha Gharge 13th JUly,2015
		//if (uNumFiles <= 5)
		//{
			for (int i = 0; i < static_cast<int>(uNumFiles); i++)//Ram: removed warning
			{
				if (0 == DragQueryFile(hDrop, i, m_szFile, MAX_PATH))
					hr = E_INVALIDARG;
				m_vszFolderPaths.push_back(m_szFile);
				//	m_vszFolderPaths.push_back(L";");
			}

		//}
		GlobalUnlock(stg.hGlobal);
		ReleaseStgMedium(&stg);
		return hr;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumShlExt::Initialize",0,0,true,SECONDLEVEL);
		return E_INVALIDARG;
	}
	return S_OK;
   
}
/**********************************************************************************************************
*  Function Name  :	QueryContextMenu
*  Description    :	Adds commands to a shortcut menu.
*  Author Name    : Neha Gharge
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_007
*  Modification	Date : 5th June,2015 Neha Gharge Adding Encrypt and Decrypt option.
**********************************************************************************************************/
STDMETHODIMP CSimpleShlExt::QueryContextMenu(
	HMENU hmenu, UINT uMenuIndex, UINT uidFirstCmd,
	UINT uidLastCmd, UINT uFlags)
{
	__try
	{
		return QueryContextMenuSEH(hmenu, uMenuIndex, uidFirstCmd, uidLastCmd, uFlags);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()", 0, 0, true, SECONDLEVEL);
		return E_INVALIDARG;
	}
	return E_INVALIDARG;
}
/**********************************************************************************************************                     
*  Function Name  :	QueryContextMenuSEH                                                     
*  Description    :	Adds commands to a shortcut menu.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_007
*  Modification	Date : 5th June,2015 Neha Gharge Adding Encrypt and Decrypt option.
**********************************************************************************************************/
STDMETHODIMP CSimpleShlExt::QueryContextMenuSEH (
    HMENU hmenu, UINT uMenuIndex, UINT uidFirstCmd,
    UINT uidLastCmd, UINT uFlags )
{
	DWORD dwNoofFiles = 0x00;
	TCHAR szWardWizScanName[512] = { 0 };
	TCHAR szEncryptName[512] = { 0 };
	TCHAR szDecryptName[512] = { 0 };
	TCHAR *szExt = NULL;
	UINT idCmd = uidFirstCmd;
	try
	{
		m_dwWardWizOpt = 0x00;

		m_dwWardWizOpt = IsFileEncrypted();
		CheckForOSOrNetworkPath();
		// If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
		if (uFlags & CMF_DEFAULTONLY || m_bIsNetworkPath)
		{
			return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
		}
		else if (uFlags & CMF_VERBSONLY)
		{
			//Issue 421 ,424 and 428 Some link file not getting right click scan option,
			//File count when link file selected is remain 1 and in start up menu 2 times option
			//is display. All three issue s resolved. changes in WrdWizSimpleShlExt.rgs file.
			return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
		}
		if (uFlags & CMF_DEFAULTONLY || m_dwProductID == 0xFFFF)
		{
			return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
		}
		else
		{
			// resolved by lalit kumawat 7-25-2015
			// to check network path and we are not providing any option for network location even scan
			dwNoofFiles = m_iTotalFolderSelected;

			if (dwNoofFiles == 1)
			{
				szExt = wcsrchr(m_szFile, '\\');
				if (szExt != NULL)
				{
					CString csFileName(szExt);
					if (csFileName == L"\\")
					{
						wsprintf(szWardWizScanName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_SCAN"), (m_szFile));
					}
					else
					{
						wsprintf(szWardWizScanName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_SCAN"), (szExt + 1));
					}
				}
				else
				{
					return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
				}
			}
			else
			{
				_tcscpy_s(szWardWizScanName, _countof(szWardWizScanName), m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_SCAN_USING_WARDWIZ"));
			}

			//uMenuIndex = 0;
			//uidFirstCmd = 0;
			DWORD dwclientOS = DetectClientOSVersion();

			switch (dwclientOS)
			{
			case WINOS_XP:
			case WINOS_XP64:
				m_hRegBmp = LoadBitmap(_Module.GetModuleInstance(),
					MAKEINTRESOURCE(IDB_BITMAP_XP));
				break;
			default:
				m_hRegBmp = LoadBitmap(_Module.GetModuleInstance(),
					MAKEINTRESOURCE(IDB_BITMAP1));
				break;
			}

			

			InsertMenu(hmenu, uMenuIndex++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
			idCmd++;

			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof mii;
			mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID;
			mii.wID = idCmd++;
			mii.fType = MFT_STRING;
			mii.dwTypeData = (TCHAR *)szWardWizScanName;
			mii.cch = (UINT)_tcslen((TCHAR *)szWardWizScanName) + 1;
			mii.fState = MFS_ENABLED;
			mii.hSubMenu = NULL;

			InsertMenuItem(hmenu, uMenuIndex++, TRUE, &mii);

			// Set the bitmap for the register item.
			if (NULL != m_hRegBmp)
				SetMenuItemBitmaps(hmenu, uMenuIndex -1 , MF_STRING | MF_BYPOSITION, m_hRegBmp, NULL);

			if(m_dwProductID != BASIC)
			{
				if (m_dwWardWizOpt == 0x00 && !m_bIsSystemPath)
				{
					if (dwNoofFiles == 1)
					{
						szExt = wcsrchr(m_szFile, '\\');
						if (szExt != NULL)
						{
							CString csFileName(szExt);
							if (csFileName == L"\\")
							{
								wsprintf(szEncryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_ENCRYPT"), (m_szFile));
							}
							else
							{
								wsprintf(szEncryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_ENCRYPT"), (szExt + 1));
							}
						}
						else
						{
							return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
						}
					}
					else
					{
						_tcscpy_s(szEncryptName, _countof(szEncryptName), m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_ENCRYPT_USING_WARDWIZ"));
					}
					InsertMenu(hmenu, uMenuIndex++, MF_STRING | MF_BYPOSITION, idCmd, szEncryptName);
					// Set the bitmap for the register item.
					if (NULL != m_hRegBmp)
						SetMenuItemBitmaps(hmenu, uMenuIndex - 1, MF_STRING | MF_BYPOSITION, m_hRegBmp, NULL);
					idCmd++;
				}
				else if (m_dwWardWizOpt == 0x01)
				{
					if (dwNoofFiles == 1)
					{
						szExt = wcsrchr(m_szFile, '\\');
						CString csFileName(szExt);
						if (csFileName == L"\\")
						{
							wsprintf(szDecryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_DECRYPT"), (m_szFile));
						}
						else
						{
							wsprintf(szDecryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_DECRYPT"), (szExt + 1));
						}
					}
					else
					{
						_tcscpy_s(szDecryptName, _countof(szDecryptName), m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_DECRYPT_USING_WARDWIZ"));
					}
					InsertMenu(hmenu, uMenuIndex++, MF_STRING | MF_BYPOSITION, idCmd, szDecryptName);

					// Set the bitmap for the register item.
					if (NULL != m_hRegBmp)
						SetMenuItemBitmaps(hmenu, uMenuIndex - 1, MF_STRING | MF_BYPOSITION, m_hRegBmp, NULL);
					idCmd++;
				}
				else if (m_dwWardWizOpt == 0x02)
				{
					if (!m_bIsSystemPath)
					{
						if (dwNoofFiles == 1)
						{
							szExt = wcsrchr(m_szFile, '\\');
							if (szExt != NULL)
							{
								CString csFileName(szExt);
								if (csFileName == L"\\")
								{
									wsprintf(szEncryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_ENCRYPT"), (m_szFile));
								}
								else
								{
									wsprintf(szEncryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_ENCRYPT"), (szExt + 1));
								}
							}
							else
							{
								return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
							}
						}
						else
						{
							_tcscpy_s(szEncryptName, _countof(szEncryptName), m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_ENCRYPT_USING_WARDWIZ"));
						}
						InsertMenu(hmenu, uMenuIndex++, MF_STRING | MF_BYPOSITION, idCmd, szEncryptName);
						// Set the bitmap for the register item.
						if (NULL != m_hRegBmp)
							SetMenuItemBitmaps(hmenu, uMenuIndex - 1, MF_STRING | MF_BYPOSITION, m_hRegBmp, NULL);
						idCmd++;
					}

					if (dwNoofFiles == 1)
					{
						szExt = wcsrchr(m_szFile, '\\');
						if (szExt != NULL)
						{
							CString csFileName(szExt);
							if (csFileName == L"\\")
							{
								wsprintf(szDecryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_DECRYPT"), (m_szFile));
							}
							else
							{
								wsprintf(szDecryptName, L"%s %s", m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_DECRYPT"), (szExt + 1));
							}
						}
						else
						{
							return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
						}
					}
					else
					{
						_tcscpy_s(szDecryptName, _countof(szDecryptName), m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_DECRYPT_USING_WARDWIZ"));
					}
					InsertMenu(hmenu, uMenuIndex++, MF_STRING | MF_BYPOSITION, idCmd, szDecryptName);
					// Set the bitmap for the register item.
					if (NULL != m_hRegBmp)
						SetMenuItemBitmaps(hmenu, uMenuIndex - 1, MF_STRING | MF_BYPOSITION, m_hRegBmp, NULL);
					idCmd++;

				}

			}

			InsertMenu(hmenu, uMenuIndex++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
			idCmd++;
			return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, (idCmd - uidFirstCmd));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumShlExt::QueryContextMenu", 0, 0, true, SECONDLEVEL);
	}
	return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 1 );
}

/**********************************************************************************************************
*  Function Name  :	GetCommandString
*  Description    :	Gets information about a shortcut menu command, including the help string and the language-independent, or canonical, name for the command.
*  Author Name    : Neha Gharge
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_008
**********************************************************************************************************/
STDMETHODIMP CSimpleShlExt::GetCommandString(
	UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	__try
	{
		return GetCommandStringSEH(idCmd, uFlags, pwReserved, pszName, cchMax);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()", 0, 0, true, SECONDLEVEL);
		return E_NOTIMPL;
	}
	return E_NOTIMPL;
}
/**********************************************************************************************************                     
*  Function Name  :	GetCommandStringSEH                                                     
*  Description    :	Gets information about a shortcut menu command, including the help string and the language-independent, or canonical, name for the command.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_008
**********************************************************************************************************/
STDMETHODIMP CSimpleShlExt::GetCommandStringSEH (
    UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax )
{

	//Neha Gharge Lnk get failed in getcommandstrings.23/4/2015
	try
	{
		if (!pszName)
			return E_NOTIMPL;

		LPWSTR wBuffer = (LPWSTR)pszName;
		if (uFlags == GCS_HELPTEXTA) {
			lstrcpynA(pszName, szHelpTextA, cchMax);
			return S_OK;
		}
		else if (uFlags == GCS_HELPTEXTW) {
			lstrcpynW(wBuffer, szHelpTextW, cchMax);
			return S_OK;
		}
		return E_NOTIMPL;


	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumShlExt::GetCommandString", 0, 0, true, SECONDLEVEL);
		return E_INVALIDARG;
	}
	return S_OK;
}

/**********************************************************************************************************
*  Function Name  :	InvokeCommand
*  Description    :	Carries out the command associated with a shortcut menu item.
*  Author Name    : Neha Gharge
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_009
*  Modification	Date : 5th June,2015 Neha Gharge Adding Encrypt and Decrypt option.
**********************************************************************************************************/
STDMETHODIMP CSimpleShlExt::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo)
{
	__try
	{
		return InvokeCommandSEH(pCmdInfo);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()", 0, 0, true, SECONDLEVEL);
		return E_INVALIDARG;
	}
	return E_INVALIDARG;
}
/**********************************************************************************************************                     
*  Function Name  :	InvokeCommand                                                     
*  Description    :	Carries out the command associated with a shortcut menu item.
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 2 Sept 2014
*  SR_NO		  : WRDWIZSHELLEXT_009
*  Modification	Date : 5th June,2015 Neha Gharge Adding Encrypt and Decrypt option.
**********************************************************************************************************/
STDMETHODIMP CSimpleShlExt::InvokeCommandSEH(LPCMINVOKECOMMANDINFO pCmdInfo)
{
	TCHAR szModuleName[MAX_PATH] = { 0 };
	try
	{
		// If lpVerb really points to a string, ignore this function call and bail out.
		if (0 != HIWORD(pCmdInfo->lpVerb))
			return E_INVALIDARG;

		// Get the command index - the only valid one is 0.
		if (!HIWORD(pCmdInfo->lpVerb))
		{
			UINT idCmd = LOWORD(pCmdInfo->lpVerb);
			// issue click on decryption click it lunching encryption dlg.
			// resolvecd by lalit kumawat 7 -25-2015
			if (m_bIsSystemPath || m_bIsNetworkPath)
			{
				if (idCmd > 1)
				{
					idCmd = idCmd + 1;
				}
			}
			switch (idCmd)
			{
			case 1: //If select First menu of wardwiz
			{

				// Message if selected folders are more than 5
				if (m_iTotalFolderSelected > 5)
				{
					//Neha Gharge 11 July,2010 
					//Issue 703: Message box appeaaring in new context
					MessageBox(pCmdInfo->hwnd, m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_TOO_MANY_ARG"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
					break;
				}

				if (!GetWardWizPathFromRegistry(szModuleName))
				{
					MessageBox(pCmdInfo->hwnd, m_objwardwizLangManager.GetString(L"IDS_APPFOLDER_NOTEXIST"), L"Vibranium", MB_ICONEXCLAMATION);
				}

				_tcscat_s(szModuleName, L"VBUSBDETECTUI.EXE");

				//_tcscat_s(szModuleName,L" -USBSCAN");

				m_vszFolderPathArguments.clear();
				m_vszFolderPathArguments.push_back(L"-CUSTOMSCAN");
				for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)//Ram: Removed warning
				{
					m_vszFolderPathArguments.push_back(m_vszFolderPaths.at(i));
				}

				LPCWSTR strArguments[7] = { L"", L"", L"", L"", L"", L"", L"" };
				int jIndex = 0;
				for (jIndex = 0; jIndex < static_cast<int>(m_vszFolderPathArguments.size()); jIndex++)//Ram: Removed warning
				{
					strArguments[jIndex] = m_vszFolderPathArguments.at(jIndex).c_str();
					m_dwlenOfArguments += static_cast<DWORD>(wcslen(strArguments[jIndex]));//Ram: Removed warning
				}
				m_dwlenOfArguments += jIndex;
				LPCWSTR Semicolon = TEXT("|");

				TCHAR szTotalArguments[10000] = { 0 };

				if (m_iTotalFolderSelected <= 5)
				{
					memset(szTotalArguments, 0x00, sizeof(szTotalArguments));
					//Ram: Removed warnings
					_tcscpy_s(szTotalArguments, strArguments[0]);
					_tcscat_s(szTotalArguments, L" ");
					_tcscat_s(szTotalArguments, strArguments[1]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[2]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[3]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[4]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[5]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[6]);
					_tcscat_s(szTotalArguments, L"|");
				}

				ShellExecute(NULL, L"Open", szModuleName, szTotalArguments, NULL, SWP_SHOWWINDOW);
			}
			break;
			case 2: //If select Sec menu of wardwiz
			{
				// Message if selected folders are more than 5
				if (m_iTotalFolderSelected > 5)
				{
					//Neha Gharge 11 July,2010 
					//Issue 703: Message box appeaaring in new context
					MessageBox(pCmdInfo->hwnd, m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_TOO_MANY_ARG"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
					break;
				}

				if (!GetWardWizPathFromRegistry(szModuleName))
				{
					MessageBox(pCmdInfo->hwnd, m_objwardwizLangManager.GetString(L"IDS_APPFOLDER_NOTEXIST"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				}

				_tcscat_s(szModuleName, L"VBUI.EXE");

				m_vszFolderPathArguments.clear();

				TCHAR szTotalArguments[6144] = { 0 };

				if (m_dwWardWizOpt == 0x00)
				{
					m_vszFolderPathArguments.push_back(L"-ENC");
					for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)//Ram: Removed warning
					{
						m_vszFolderPathArguments.push_back(m_vszFolderPaths.at(i));
					}

					LPCWSTR strArguments[7] = { L"", L"", L"", L"", L"", L"", L"" };
					int jIndex = 0;
					for (jIndex = 0; jIndex < static_cast<int>(m_vszFolderPathArguments.size()); jIndex++)
					{
						strArguments[jIndex] = m_vszFolderPathArguments.at(jIndex).c_str();
						m_dwlenOfArguments += static_cast<DWORD>(wcslen(strArguments[jIndex]));
					}
					m_dwlenOfArguments += jIndex;
					LPCWSTR Semicolon = TEXT("|");



					if (m_iTotalFolderSelected <= 5)
					{
						memset(szTotalArguments, 0x00, sizeof(szTotalArguments));
						_tcscpy_s(szTotalArguments, strArguments[0]);
						_tcscat_s(szTotalArguments, L" ");
						_tcscat_s(szTotalArguments, strArguments[1]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[2]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[3]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[4]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[5]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[6]);
						_tcscat_s(szTotalArguments, L"|");
					}

					ShellExecute(NULL, L"Open", szModuleName, szTotalArguments, NULL, SWP_SHOWWINDOW);
				}
				else if	(m_dwWardWizOpt == 0x01)
				{
					m_vszFolderPathArguments.push_back(L"-DEC");
					for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)
					{
						m_vszFolderPathArguments.push_back(m_vszFolderPaths.at(i));
					}

					LPCWSTR strArguments[7] = { L"", L"", L"", L"", L"", L"", L"" };
					int jIndex = 0;
					for (jIndex = 0; jIndex < static_cast<int>(m_vszFolderPathArguments.size()); jIndex++)
					{
						strArguments[jIndex] = m_vszFolderPathArguments.at(jIndex).c_str();
						m_dwlenOfArguments += static_cast<DWORD>(wcslen(strArguments[jIndex]));
					}
					m_dwlenOfArguments += jIndex;
					LPCWSTR Semicolon = TEXT("|");



					if (m_iTotalFolderSelected <= 5)
					{
						memset(szTotalArguments, 0x00, sizeof(szTotalArguments));
						_tcscpy_s(szTotalArguments, strArguments[0]);
						_tcscat_s(szTotalArguments, L" ");
						_tcscat_s(szTotalArguments, strArguments[1]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[2]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[3]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[4]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[5]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[6]);
						_tcscat_s(szTotalArguments, L"|");
					}

					ShellExecute(NULL, L"Open", szModuleName, szTotalArguments, NULL, SWP_SHOWWINDOW);
				}
				else if (m_dwWardWizOpt == 0x02)
				{
					m_vszFolderPathArguments.push_back(L"-ENC");
					for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)
					{
						m_vszFolderPathArguments.push_back(m_vszFolderPaths.at(i));
					}

					LPCWSTR strArguments[7] = { L"", L"", L"", L"", L"", L"", L"" };
					int jIndex = 0;
					for (jIndex = 0; jIndex < static_cast<int>(m_vszFolderPathArguments.size()); jIndex++)
					{
						strArguments[jIndex] = m_vszFolderPathArguments.at(jIndex).c_str();
						m_dwlenOfArguments += static_cast<DWORD>(wcslen(strArguments[jIndex]));
					}
					m_dwlenOfArguments += jIndex;
					LPCWSTR Semicolon = TEXT("|");



					if (m_iTotalFolderSelected <= 5)
					{
						memset(szTotalArguments, 0x00, sizeof(szTotalArguments));
						_tcscpy_s(szTotalArguments, strArguments[0]);
						_tcscat_s(szTotalArguments, L" ");
						_tcscat_s(szTotalArguments, strArguments[1]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[2]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[3]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[4]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[5]);
						_tcscat_s(szTotalArguments, L"|");
						_tcscat_s(szTotalArguments, strArguments[6]);
						_tcscat_s(szTotalArguments, L"|");
					}

					ShellExecute(NULL, L"Open", szModuleName, szTotalArguments, NULL, SWP_SHOWWINDOW);
				}

			}
			break;
			case 3: //If select third menu of wardwiz
			{
				// Message if selected folders are more than 5
				if (m_iTotalFolderSelected > 5)
				{
					//Neha Gharge 11 July,2010 
					//Issue 703: Message box appeaaring in new context
					MessageBox(pCmdInfo->hwnd, m_objwardwizLangManager.GetString(L"IDS_SHELL_EXT_TOO_MANY_ARG"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
					break;
				}

				if (!GetWardWizPathFromRegistry(szModuleName))
				{
					MessageBox(pCmdInfo->hwnd, m_objwardwizLangManager.GetString(L"IDS_APPFOLDER_NOTEXIST"), m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				}

				_tcscat_s(szModuleName, L"VBUI.EXE");

				m_vszFolderPathArguments.clear();

				TCHAR szTotalArguments[10000] = { 0 };
				m_vszFolderPathArguments.push_back(L"-DEC");
				for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)
				{
					m_vszFolderPathArguments.push_back(m_vszFolderPaths.at(i));
				}

				LPCWSTR strArguments[7] = { L"", L"", L"", L"", L"", L"", L"" };
				int jIndex = 0;
				for (jIndex = 0; jIndex < static_cast<int>(m_vszFolderPathArguments.size()); jIndex++)
				{
					strArguments[jIndex] = m_vszFolderPathArguments.at(jIndex).c_str();
					m_dwlenOfArguments += static_cast<DWORD>(wcslen(strArguments[jIndex]));
				}
				m_dwlenOfArguments += jIndex;
				LPCWSTR Semicolon = TEXT("|");



				if (m_iTotalFolderSelected <= 5)
				{
					memset(szTotalArguments, 0x00, sizeof(szTotalArguments));
					_tcscpy_s(szTotalArguments, strArguments[0]);
					_tcscat_s(szTotalArguments, L" ");
					_tcscat_s(szTotalArguments, strArguments[1]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[2]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[3]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[4]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[5]);
					_tcscat_s(szTotalArguments, L"|");
					_tcscat_s(szTotalArguments, strArguments[6]);
					_tcscat_s(szTotalArguments, L"|");
				}

				ShellExecute(NULL, L"Open", szModuleName, szTotalArguments, NULL, SWP_SHOWWINDOW);

			}
			break;

			default:
				return E_INVALIDARG;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumShlExt::InvokeCommand", 0, 0, true, SECONDLEVEL);
		return E_INVALIDARG;
	}
	return S_OK;
}

/***************************************************************************************************
*  Function Name  : GetWardWizPathFromRegistry
*  Description    : Get the path where app folder is exist
*  Author Name    : Neha Gharge
*  SR_NO		  : WRDWIZSHELLEXT_010
*  Date           : 2 Sept 2014
****************************************************************************************************/
bool CSimpleShlExt::GetWardWizPathFromRegistry(TCHAR *szModuleName)
{
	bool bReturn = false;
	__try
	{
		return GetWardWizPathFromRegistrySEH(szModuleName);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
/***************************************************************************************************                    
*  Function Name  : GetWardWizPathFromRegistrySEH                                                     
*  Description    : Get the path where app folder is exist
*  Author Name    : Neha Gharge
*  SR_NO		  : WRDWIZSHELLEXT_010
*  Date           : 2 Sept 2014
****************************************************************************************************/
bool CSimpleShlExt::GetWardWizPathFromRegistrySEH(TCHAR *szModuleName)
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	try
	{

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_csProdRegKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
			return false;

		DWORD	dwSize = 511;
		DWORD	dwType = 0x00;

		RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize);
		RegCloseKey(hSubKey);
		hSubKey = NULL;

		if (_tcslen(szModulePath) > 0)
		{
			_tcscpy(szModuleName, szModulePath);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumShlExt::GetWardwizPathFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************
Function Name  : DetectClientOSVersion
Description    : Return client machine os version
Author Name    : Neha gharge
Date           : 22th April 2015
****************************************************************************/
DWORD CSimpleShlExt::DetectClientOSVersion()
{
	DWORD dwRet = 0x00;
	__try
	{
		return DetectClientOSVersionSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}
/***************************************************************************
Function Name  : DetectClientOSVersionSEH
Description    : Return client machine os version
Author Name    : Neha gharge
Date           : 22th April 2015
****************************************************************************/
DWORD CSimpleShlExt::DetectClientOSVersionSEH()
{
	try
	{
		OSVERSIONINFO OSversion;

		OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		::GetVersionEx(&OSversion);

		switch (OSversion.dwPlatformId)
		{
		case VER_PLATFORM_WIN32s:
			AddLogEntry(L"Client version is VER_PLATFORM_WIN32s", 0, 0, true, FIRSTLEVEL);
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if (OSversion.dwMinorVersion == 0)
				return WINOS_95;
			else
				if (OSversion.dwMinorVersion == 10)
					return WINOS_98;
				else
					if (OSversion.dwMinorVersion == 90)
						return WINOSUNKNOWN_OR_NEWEST;

		case VER_PLATFORM_WIN32_NT:
			if (OSversion.dwMajorVersion == 5 && OSversion.dwMinorVersion == 0)
				return WINOS_2000;
			else
				if (OSversion.dwMajorVersion == 5 && OSversion.dwMinorVersion == 1)
					return WINOS_XP;
				else
					if (OSversion.dwMajorVersion <= 4)
						return WINOS_NT;
					else
						if (OSversion.dwMajorVersion == 6 && OSversion.dwMinorVersion == 0)
							return WINOS_VISTA;
						else
							if (OSversion.dwMajorVersion == 6 && OSversion.dwMinorVersion == 1)
								return WINOS_WIN7;
							else
								if (OSversion.dwMajorVersion == 6 && OSversion.dwMinorVersion == 2)
									return WINOS_WIN8;
								else
									if (OSversion.dwMajorVersion == 6 && OSversion.dwMinorVersion == 3)
										return WINOS_WIN8_1;
									else
										if (OSversion.dwMajorVersion == 5 && OSversion.dwMinorVersion == 2) // solved by lalit ,issue  handle the xp64 Os version
											return WINOS_XP64;
										else
											return WINOSUNKNOWN_OR_NEWEST;

		default:
			return WINOSUNKNOWN_OR_NEWEST;


		}
		return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::DetectClientOSVersion()",0,0,true,SECONDLEVEL);
	}
	return 0;

}

/***************************************************************************
Function Name  : IsFileEncrypt
Description    : Return bool true if it is a file is encrypted
Author Name    : Neha gharge
Date           : 22th April 2015
****************************************************************************/
DWORD CSimpleShlExt::IsFileEncrypted()
{
	DWORD dwRet = 0x00;
	__try
	{
		 return IsFileEncryptedSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::IsFileEncrypted()", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}
/***************************************************************************
Function Name  : IsFileEncrypt SEH
Description    : Return bool true if it is a file is encrypted
Author Name    : Neha gharge
Date           : 22th April 2015
****************************************************************************/
DWORD CSimpleShlExt::IsFileEncryptedSEH()
{
	DWORD dwRet = 0x00;
	try
	{
		
		DWORD dwTotalFiles = 0x00, dwDecryptFile = 0x00, dwEncryptFile = 0x00;
		TCHAR * szExt = NULL;
		TCHAR szFilepath[512] = { 0 };
		wstring wstrFilePath;
		m_bIsSystemPath = false;

		GetEnviormentVariablesForAllMachine();

		dwTotalFiles = static_cast<DWORD>(m_vszFolderPaths.size());

		for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)
		{
			wstrFilePath = m_vszFolderPaths.at(i);

			
			_tcscpy_s(szFilepath, _countof(szFilepath), wstrFilePath.c_str());
			dwRet = GetFileAttributes(szFilepath);
			if (dwRet == FILE_ATTRIBUTE_DIRECTORY)
				return 0x02;
			
			
			// issue :1) if file having .wwiz extension in samll later then it now showing decryption option when we right click on the selected file.
			// issue :2) if file does not have any extension in such case during right click explorer getting crash.
			// resolved by Lalit kumawat 7-6-2015.
			szExt = wcsrchr(szFilepath, '\\');
			szExt = wcsrchr(szExt, '.');

			if (!szExt)
			{
				return 0x02;
			}

			if (_tcscmp(szExt, L".AK") == 0x00 || _tcscmp(szExt, L".ak") == 0x00)
			{
				dwDecryptFile++;
			}
			else
			{
				dwEncryptFile++;
			}
		}

		if (dwDecryptFile == dwTotalFiles)
		{
			return 0x01;
		}
		else if (dwEncryptFile == dwTotalFiles)
		{
			return 0x00;
		}
		else
		{
			return 0x02;
		}
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSimpleShlExt::IsFileEncrypted()", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***************************************************************************
Function Name  : GetEnviormentVariablesForAllMachine
Description    : this function provides functionality to find the all system defined path
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
void CSimpleShlExt::GetEnviormentVariablesForAllMachine()
{
	
	try
	{
		// issue :- sometime on vista32 bit machine not showing ecryption option on right click
		// resolved by lalit kumawat
		// solution  memory clean up added and update the AddLog entry function previously it was not working at all.

		GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), m_szProgramData, MAX_PATH);//program data

		GetEnvironmentVariable(TEXT("USERPROFILE"), m_szUserProfile, MAX_PATH);

		GetEnvironmentVariable(TEXT("windir"), m_szWindow, MAX_PATH);//windows

		GetEnvironmentVariable(TEXT("APPDATA"), m_szAppData, MAX_PATH);

		GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITECTURE"), m_szAppData4, MAX_PATH);

		GetEnvironmentVariable(TEXT("SystemDrive"), m_szDriveName, MAX_PATH);

		GetModulePath(m_szModulePath, MAX_PATH);

		GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITEW6432"), m_szArchitecture, MAX_PATH);

		GetEnvironmentVariable(TEXT("ProgramFiles(x86)"), m_szProgramFilex86, MAX_PATH);

		GetEnvironmentVariable(TEXT("ProgramFiles"), m_szProgramFile, MAX_PATH);
		// resolved by lalit kumawat
		// issue : in 32 bit operating system user getting access denied for desktop path
		if (!_tcscmp(m_szProgramFilex86, L""))
		{
			_tcscpy_s(m_szProgramFilex86, m_szProgramFile);

		}
		
	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in GetEnviormentVariablesForAllMachine", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : IsPathBelongsToOSReservDirectory
Description    : this function provides functionality to check is selected path belongs to system drive or folder
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
bool CSimpleShlExt::IsPathBelongsToOSReservDirectory(wstring wsFilefolderPath)
{
	bool bRet = false;
	
	TCHAR szRegDBPath[20] = L"VBUSERREG.DB";
	TCHAR szOSPagefile[MAX_PATH] = { 0 };
	TCHAR szOShiberfile[MAX_PATH] = { 0 };
	TCHAR szOSSwapfile[MAX_PATH] = { 0 };
	TCHAR szOSconfigFile[MAX_PATH] = { 0 };
	TCHAR szOSFilebootmgr[MAX_PATH] = { 0 };
	TCHAR szOSAUTOEXEC[MAX_PATH] = { 0 };
	TCHAR szOSawayFile[MAX_PATH] = { 0 };
	TCHAR szOSBOOTNXT[MAX_PATH] = { 0 };
	TCHAR szOSgone[MAX_PATH] = { 0 };
	TCHAR szOSBOOTSECT[MAX_PATH] = { 0 };
	TCHAR szOSSystemVolumInfo[MAX_PATH] = { 0 };
	TCHAR szOSIO[MAX_PATH] = { 0 };
	TCHAR szOSMSDOS[MAX_PATH] = { 0 };
	TCHAR szOSNTDETECT[MAX_PATH] = { 0 };
	TCHAR szOSntldr[MAX_PATH] = { 0 };

	try
	{
		// registration db file need to avoid during encryption.
		// resolved by lalit kumawat 7-20-2015
		swprintf_s(szOSPagefile, _countof(szOSPagefile), L"%s\\pagefile.sys", m_szDriveName);
		swprintf_s(szOShiberfile, _countof(szOShiberfile), L"%s\\hiberfil.sys", m_szDriveName);
		swprintf_s(szOSSwapfile, _countof(szOSSwapfile), L"%s\\swapfile.sys", m_szDriveName);
		swprintf_s(szOSconfigFile, _countof(szOSconfigFile), L"%s\\config.sys", m_szDriveName);
		swprintf_s(szOSFilebootmgr, _countof(szOSFilebootmgr), L"%s\\bootmgr", m_szDriveName);
		swprintf_s(szOSAUTOEXEC, _countof(szOSAUTOEXEC), L"%s\\AUTOEXEC.BAT", m_szDriveName);
		swprintf_s(szOSawayFile, _countof(szOSawayFile), L"%s\\away.dat", m_szDriveName);
		swprintf_s(szOSBOOTNXT, _countof(szOSBOOTNXT), L"%s\\BOOTNXT", m_szDriveName);
		swprintf_s(szOSgone, _countof(szOSgone), L"%s\\gone.dat", m_szDriveName);
		swprintf_s(szOSBOOTSECT, _countof(szOSBOOTSECT), L"%s\\BOOTSECT.BAK", m_szDriveName);
		swprintf_s(szOSSystemVolumInfo, _countof(szOSSystemVolumInfo), L"%s\\System Volume Information", m_szDriveName);
		swprintf_s(szOSIO, _countof(szOSIO), L"%s\\IO.SYS", m_szDriveName);
		swprintf_s(szOSMSDOS, _countof(szOSMSDOS), L"%s\\MSDOS.SYS", m_szDriveName);
		swprintf_s(szOSNTDETECT, _countof(szOSNTDETECT), L"%s\\NTDETECT.COM", m_szDriveName);
		swprintf_s(szOSntldr, _countof(szOSntldr), L"%s\\ntldr", m_szDriveName);

		if ((wsFilefolderPath.find(m_szProgramData) != -1) && m_szProgramData !=L"")
		{
			return true;
		}
		else if ((wsFilefolderPath.find(m_szWindow) != -1) && m_szWindow != L"")
		{
			return true;
		}
		else if ((wsFilefolderPath.find(m_szModulePath) != -1) && m_szModulePath != L"")
		{
			return true;
		}
		else if ((wsFilefolderPath.find(m_szProgramFilex86) != -1) && m_szProgramFilex86 != L"")
		{
			return true;
		}
		else if ((wsFilefolderPath.find(m_szProgramFile) != -1) && !wsFilefolderPath.empty())
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szRegDBPath) != -1) || wsFilefolderPath.c_str() == szRegDBPath)
		{
			return true;
		}
		else if (IsFileIsProtected(wsFilefolderPath))
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSPagefile) != -1) || wsFilefolderPath.c_str() == szOSPagefile)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOShiberfile) != -1) || wsFilefolderPath.c_str() == szOShiberfile)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSSwapfile) != -1) || wsFilefolderPath.c_str() == szOSSwapfile)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSconfigFile) != -1) || wsFilefolderPath.c_str() == szOSconfigFile)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSFilebootmgr) != -1) || wsFilefolderPath.c_str() == szOSFilebootmgr)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSAUTOEXEC) != -1) || wsFilefolderPath.c_str() == szOSAUTOEXEC)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSawayFile) != -1) || wsFilefolderPath.c_str() == szOSawayFile)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSBOOTNXT) != -1) || wsFilefolderPath.c_str() == szOSBOOTNXT)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSgone) != -1) || wsFilefolderPath.c_str() == szOSgone)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSBOOTSECT) != -1) || wsFilefolderPath.c_str() == szOSBOOTSECT)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSSystemVolumInfo) != -1) || wsFilefolderPath.c_str() == szOSSystemVolumInfo)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSIO) != -1) || wsFilefolderPath.c_str() == szOSIO)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSMSDOS) != -1) || wsFilefolderPath.c_str() == szOSMSDOS)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSNTDETECT) != -1) || wsFilefolderPath.c_str() == szOSNTDETECT)
		{
			return true;
		}
		else if ((wsFilefolderPath.find(szOSntldr) != -1) || wsFilefolderPath.c_str() == szOSntldr)
		{
			return true;
		}
		else if (wsFilefolderPath.c_str() == m_szDriveName)
		{
			return true;
		}
		

		DWORD uDriveMask = GetLogicalDrives();
		if (uDriveMask != 0)
		{
			TCHAR szDrive[] = _T(" A:");
			bool bDriveMatch = false;
			wstring  wstempPath;
			
			while (uDriveMask)
			{
				// Use the bitwise AND, 1â€"available, 0-not available
				wstempPath = wsFilefolderPath;
				TCHAR  *wsAvlblDrv = L"";
				TCHAR szDriveName[8] = { 0 };
				// issue: hiding encryption option on System volume information directory in all directories
				// resolved by lalit kumawat 7-29-2015
				TCHAR szSystemVolumInfoDir[MAX_PATH] = {0};

				if (uDriveMask & 1)
				{
					wsAvlblDrv = szDrive;
					
					swprintf_s(szDriveName, 7, L"%s\\", wsAvlblDrv);
					swprintf_s(szSystemVolumInfoDir, _countof(szSystemVolumInfoDir), L"%s\\System Volume Information", szDrive);

					wstempPath = L" ";
					wstempPath += wstempPath.c_str();

					if (wstempPath.c_str() == szDriveName || wstempPath.c_str() == szSystemVolumInfoDir)
						{
							bDriveMatch =  true;
							break;
						}
				}
					
				// increment, check next drive
				++szDrive[1];
				// shift the bitmask binary right
				uDriveMask >>= 1;

			}

			if (bDriveMatch)
			{
				return true;
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in IsPathBelongsToOSReservDirectory", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/***************************************************************************
Function Name  : IsNetworkPathSelected
Description    : this function provides functionality to check is selected path belongs to network drive or folder
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
bool CSimpleShlExt::IsNetworkPathSelected(wstring wsPath)
{
	bool bRet = false;
	TCHAR szSelectedPath[3 * MAX_PATH] = { 0 };
	
	try
	{
		swprintf_s(szSelectedPath, _countof(szSelectedPath), L"%s", wsPath.c_str());

		bRet = PathIsNetworkPath(szSelectedPath)?true:false;
		
	}
	catch (...)
	{
		bRet = false;
		AddLogEntry(L">>> Exception in IsNetworkPathSelected", 0, 0, true, SECONDLEVEL);
	}
	
	return bRet;
}

/***************************************************************************
Function Name  : IsFileIsProtected
Description    : this function provides functionality to check is selected path belongs to os protected file or folder
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
bool CSimpleShlExt::IsFileIsProtected(wstring wsPath)
{
	bool bRet = false;
	TCHAR szSelectedPath[3 * MAX_PATH] = { 0 };

	try
	{
		swprintf_s(szSelectedPath, _countof(szSelectedPath), L"%s", wsPath.c_str());

		DWORD dwResult = 0;
		dwResult = SfcIsFileProtected(NULL, szSelectedPath);

		if (dwResult)
		{
			bRet = true;
			TCHAR szLogEntry[3 * MAX_PATH] = { 0 };

			swprintf_s(szLogEntry, _countof(szLogEntry), L">>> Protected file Path %s", szSelectedPath);
			AddLogEntry(szLogEntry, 0, 0, true, SECONDLEVEL);
		}

	}
	catch (...)
	{
		bRet = false;
		AddLogEntry(L">>> Exception in IsFileIsProtected", 0, 0, true, SECONDLEVEL);
	}

	return bRet;
}

/***************************************************************************
Function Name  : CheckForOSOrNetworkPath
Description    : wrapper function to check os or network path
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
void CSimpleShlExt::CheckForOSOrNetworkPath()
{
	for (int i = 0; i < static_cast<int>(m_vszFolderPaths.size()); i++)
	{
		wstring wstrFilePath = L"";

		wstrFilePath = m_vszFolderPaths.at(i);

		if (m_bIsSystemPath || m_bIsSystemPath)
			break;
		
	      m_bIsNetworkPath = IsNetworkPathSelected(wstrFilePath);
			if (!m_bIsNetworkPath)
				m_bIsSystemPath = IsPathBelongsToOSReservDirectory(wstrFilePath);
			else
				m_bIsSystemPath = m_bIsNetworkPath;
		
	 }
}