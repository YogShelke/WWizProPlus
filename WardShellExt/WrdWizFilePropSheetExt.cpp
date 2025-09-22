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
#include "StdAfx.h"
#include "WrdWizFilePropSheetExt.h"
#include "iTinRegWrapper.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

HBITMAP			g_hBgBitmap = NULL;
bool			g_bSafe	= true;

#define MAX_ALLOWED_PATH	0x22
/***************************************************************************************************
*  Function Name  : CFilePropSheetExt
*  Description    : Const'r
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
CFilePropSheetExt::CFilePropSheetExt() : m_cRef(1),
    pszPageTitle(L"Vibranium")
{
}

/***************************************************************************************************
*  Function Name  : ~CFilePropSheetExt
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
CFilePropSheetExt::~CFilePropSheetExt()
{
}

/***************************************************************************************************
*  Function Name  : GetSelectedFile
*  Description    : Function which returns selected file.
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
PCWSTR CFilePropSheetExt::GetSelectedFile()
{
    return this->m_szSelectedFile;
}

#define EXT_POINTER_PROP            L"Vibranium"

INT_PTR CALLBACK FilePropPageDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
UINT CALLBACK FilePropPageCallbackProc(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

#pragma region IShellExtInit
/***************************************************************************************************
*  Function Name  : GetSelectedFile
*  Description    : Initialize the context menu extension.
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
IFACEMETHODIMP CFilePropSheetExt::Initialize(
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{
    if (NULL == pDataObj)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stm;

    // The pDataObj pointer contains the objects being acted upon. In this 
    // example, we get an HDROP handle for enumerating the selected files and 
    // folders.
    if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
    {
        // Get an HDROP handle.
        HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
        if (hDrop != NULL)
        {
            // Determine how many files are involved in this operation. This 
            // code sample displays the custom context menu item when only 
            // one file is selected. 
            UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
            if (nFiles == 1)
            {
                // Get the path of the file.
                if (0 != DragQueryFile(hDrop, 0, m_szSelectedFile, 
                    ARRAYSIZE(m_szSelectedFile)))
                {
                    hr = S_OK;
                }

				//Selected file should be in Fixed driver (or) on removable devices.
				CString csFilePath(m_szSelectedFile);
				if (GetDriveType((LPCWSTR)csFilePath.Left(2)) != DRIVE_REMOVABLE &&
					GetDriveType((LPCWSTR)csFilePath.Left(2)) != DRIVE_FIXED)
				{
					hr = E_FAIL;
				}
            }

            GlobalUnlock(stm.hGlobal);
        }

        ReleaseStgMedium(&stm);
    }

	//Ram: Get here a Product ID to check for product version
	//CWardwizLangManager objLangManager;
	m_csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
	m_dwProductID = m_objwardwizLangManager.GetSelectedProductID();

	m_csProductEdition = _T("Vibranium");
	switch (m_dwProductID)
	{
	case BASIC:
		m_csProductEdition += _T("Basic");
		break;
	case ESSENTIAL:
		m_csProductEdition += _T("Essential");
		break;
	case PRO:
		m_csProductEdition += _T("Provantage");
		break;
	}

	m_csProdVersion = GetProdutVersion();
	m_csDBVersion = GetDatabaseVersion();
	m_csScanEngineVersion = GetScanEngineVersion();

	g_bSafe = true;
	g_hBgBitmap = LoadBitmapResource(IDB_BITMAP_WARDWIZ);
    // If any value other than S_OK is returned from the method, the property 
    // sheet is not displayed.
    return hr;
}

#pragma endregion


#pragma region IShellPropSheetExt

/***************************************************************************************************
*  Function Name  : AddPages
*  Description    : Initialize the context menu extension.
Adds one or more pages to a property sheet that the Shell
displays for a file object. The Shell calls this method for
each property sheet handler registered to the file type.
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
IFACEMETHODIMP CFilePropSheetExt::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage,
    LPARAM lParam)
{
    // Create a property sheet page.
    PROPSHEETPAGE psp = { sizeof(psp) };
    psp.dwFlags = PSP_USETITLE | PSP_USECALLBACK;
	psp.hInstance = _Module.GetModuleInstance();
    psp.pszTemplate = MAKEINTRESOURCE(IDD_FILE_PROPPAGE);
    psp.pszIcon = NULL;
    psp.pszTitle = this->pszPageTitle;
    psp.pfnDlgProc = FilePropPageDlgProc;
    psp.pcRefParent = NULL;
    psp.pfnCallback = FilePropPageCallbackProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);

    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&psp);
    if (hPage == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // The property sheet page is then added to the property sheet by calling 
    // the callback function (LPFNADDPROPSHEETPAGE pfnAddPage) passed to 
    // IShellPropSheetExt::AddPages.
    if (pfnAddPage(hPage, lParam))
    {
        // By default, after AddPages returns, the shell releases its 
        // IShellPropSheetExt interface and the property page cannot access the
        // extension object. However, it is sometimes desirable to be able to use 
        // the extension object, or some other object, from the property page. So 
        // we increase the reference count and maintain this object until the 
        // page is released in PropPageCallbackProc where we call Release upon 
        // the extension.
        this->AddRef();
    }
    else
    {
        DestroyPropertySheetPage(hPage);
        return E_FAIL;
    }

    // If any value other than S_OK is returned from the method, the property 
    // sheet is not displayed.
    return S_OK;
}

/***************************************************************************************************
*  Function Name  : ReplacePage
*  Description    : Function which replace already existing page.
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
IFACEMETHODIMP CFilePropSheetExt::ReplacePage(UINT uPageID,
    LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam)
{
    // The method is not used.
    return E_NOTIMPL;
}
#pragma endregion

/***************************************************************************************************
*  Function Name  : FilePropPageDlgProc
*  Description    : Processes messages for the property page.
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
INT_PTR CALLBACK FilePropPageDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch (uMsg)
		{
			case WM_INITDIALOG:
			{
			// Get the pointer to the property sheet page object. This is 
			// contained in the LPARAM of the PROPSHEETPAGE structure.
			LPPROPSHEETPAGE pPage = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
			if (pPage != NULL)
			{
				// Access the property sheet extension from property page.
				CFilePropSheetExt *pExt = reinterpret_cast<CFilePropSheetExt *>(pPage->lParam);
				if (pExt != NULL)
				{
					CString csText = pExt->m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODEDITION_TEXT");
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_PROEDITION), csText.Trim() + L":");

					csText = pExt->m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODVERSION_TEXT");
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_PROVERSION), csText.Trim() + L":");

					csText = pExt->m_objwardwizLangManager.GetString(L"IDS_STATIC_ABOUTGUI_DBVERSION");
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_DBVERSION), csText.Trim());

					csText = pExt->m_objwardwizLangManager.GetString(L"IDS_SCANVERSION_TEXT");
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_SCANVERSION), csText.Trim());

					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_PROEDRIGHT), pExt->m_csProductEdition);
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_PROVERSRIGHT), pExt->m_csProdVersion);
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_DBVERSIONRIGHT), pExt->m_csDBVersion);
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_SCNVERSIONRIGHT), pExt->m_csScanEngineVersion);

					CString csFilePath = pExt->GetSelectedFile();
					CString csName = csFilePath.Right(csFilePath.GetLength() - csFilePath.ReverseFind(L'\\') - 1);

					csText = pExt->m_objwardwizLangManager.GetString(L"IDS_AUTO_RUN_SCAN_FILE_NAME");
					CString csFileName = csText.Trim() + L"  :  " + pExt->GetShortFilePath(csName);
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_FILE_NAME), csFileName);
					pExt->MakeBold(GetDlgItem(hWnd, IDC_STATIC_FILE_NAME));

					csText = pExt->m_objwardwizLangManager.GetString(L"IDS_BUTTON_CLEAN");
					SetWindowText(GetDlgItem(hWnd, IDC_STATIC_FILE_STATUS), csText.MakeUpper().Trim());
					if (pExt->StartScanUsingService(csFilePath))
					{
						g_bSafe = false;
						csText = pExt->m_objwardwizLangManager.GetString(L"IDS_MALICIOUS_TEXT");
						SetWindowText(GetDlgItem(hWnd, IDC_STATIC_FILE_STATUS), csText.MakeUpper().Trim());
					}
					
					pExt->MakeBold(GetDlgItem(hWnd, IDC_STATIC_FILE_STATUS));

					// Store the object pointer with this particular page dialog.
					SetProp(hWnd, EXT_POINTER_PROP, static_cast<HANDLE>(pExt));
				}
			}
			return TRUE;
			}

			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case IDC_CHANGEPROP_BUTTON:
					// User clicks the "Simulate Property Changing" button...
					// Simulate property changing. Inform the property sheet to 
					// enable the Apply button.
					MessageBox(GetParent(hWnd), L"Submit button pressed", L"Vibranium", MB_ICONINFORMATION);
					return TRUE;
				}
			}
				break;

			case WM_NOTIFY:
			{
				switch ((reinterpret_cast<LPNMHDR>(lParam))->code)
				{
				case PSN_APPLY:
					// The PSN_APPLY notification code is sent to every page in the 
					// property sheet to indicate that the user has clicked the OK, 
					// Close, or Apply button and wants all changes to take effect. 
					// Get the property sheet extension object pointer that was 
					// stored in the page dialog (See the handling of WM_INITDIALOG 
					// in FilePropPageDlgProc).
					CFilePropSheetExt *pExt = static_cast<CFilePropSheetExt *>(
						GetProp(hWnd, EXT_POINTER_PROP));

					// Access the property sheet extension object.
					// ...
					// Return PSNRET_NOERROR to allow the property dialog to close if 
					// the user clicked OK.
					SetWindowLong(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

					return TRUE;
				}
			}
				break;
			case WM_DESTROY:
			{
				// Remove the EXT_POINTER_PROP property from the page. 
				// The EXT_POINTER_PROP property stored the pointer to the 
				// CFilePropSheetExt object.
				RemoveProp(hWnd, EXT_POINTER_PROP);
				
				if (g_hBgBitmap != NULL)
				{
					DeleteObject(g_hBgBitmap);
				}
				return TRUE;
			}
			case WM_CTLCOLORSTATIC:
			{
				HDC hdc = (HDC)wParam;
				
				int iCntrID = ::GetDlgCtrlID((HWND)lParam);
				if (iCntrID == IDC_STATIC_FILE_STATUS)
				{
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor(hdc, RGB(40, 106, 0));
					if (!g_bSafe)
					{
						SetTextColor(hdc, RGB(225, 6, 6));
					}
					return (LRESULT)GetStockObject(NULL_BRUSH);
				}

				if (iCntrID == IDC_STATIC_PROEDITION ||
					iCntrID == IDC_STATIC_PROEDRIGHT ||
					iCntrID == IDC_STATIC_PROVERSION ||
					iCntrID == IDC_STATIC_PROVERSRIGHT ||
					iCntrID == IDC_STATIC_DBVERSION ||
					iCntrID == IDC_STATIC_SCNVERSIONRIGHT ||
					iCntrID == IDC_STATIC_DBVERSIONRIGHT ||
					iCntrID == IDC_STATIC_SCANVERSION ||
					iCntrID == IDC_STATIC_FILE_NAME)
				{
					SetBkMode((HDC)wParam, TRANSPARENT);
					return (LRESULT)GetStockObject(NULL_BRUSH);
				}
			}
			break;
			case WM_PAINT:
			{
				PAINTSTRUCT     ps;
				HDC             hdc;
				hdc = BeginPaint(hWnd, &ps);

				BITMAP          bitmap;
				HDC             hdcMem;
				HGDIOBJ         oldBitmap;

				hdcMem = CreateCompatibleDC(hdc);
			
				if (g_hBgBitmap != NULL)
				{
					oldBitmap = SelectObject(hdcMem, g_hBgBitmap);
					GetObject(g_hBgBitmap, sizeof(bitmap), &bitmap);
					BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
					SelectObject(hdcMem, oldBitmap);
				}

				DeleteDC(hdcMem);

				EndPaint(hWnd, &ps);
			}
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in FilePropPageDlgProc", 0, 0, true, SECONDLEVEL);
	}
    return FALSE; // Let system deal with other messages
}

/***************************************************************************************************
*  Function Name  : FilePropPageCallbackProc
*  Description    : 
Specifies an application-defined callback function that a property
sheet calls when a page is created and when it is about to be
destroyed. An application can use this function to perform
initialization and cleanup operations for the page.
*  Author Name    : Ram Shelke
*  Date			  : 21 Nov 2017
****************************************************************************************************/
UINT CALLBACK FilePropPageCallbackProc(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    switch(uMsg)
    {
    case PSPCB_CREATE:
        {
            // Must return TRUE to enable the page to be created.
            return TRUE;
        }
    case PSPCB_RELEASE:
        {
            // When the callback function receives the PSPCB_RELEASE notification, 
            // the ppsp parameter of the PropSheetPageProc contains a pointer to 
            // the PROPSHEETPAGE structure. The lParam member of the PROPSHEETPAGE 
            // structure contains the extension pointer which can be used to 
            // release the object.
            // Release the property sheet extension object. This is called even 
            // if the property page was never actually displayed.
			CFilePropSheetExt *pExt = reinterpret_cast<CFilePropSheetExt *>(ppsp->lParam);
            if (pExt != NULL)
            {
                pExt->Release();
            }
        }
        break;
    }

    return FALSE;
}


/**********************************************************************************************************
*  Function Name  :	StartScanUsingService
*  Description    :	Used named pipe to give signal to service to start scanning .
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 22 Nov 2017
**********************************************************************************************************/
bool CFilePropSheetExt::StartScanUsingService(CString csFilePath)
{
	try
	{
		if (!PathFileExists(csFilePath))
			return false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SCAN_FILE;
		wcscpy_s(szPipeData.szFirstParam, csFilePath);

		CISpyCommunicator objCom(SERVICE_SERVER, true, 0x02);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to SendData in CFilePropSheetExt::StartScanUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CFilePropSheetExt::ShutDownScanning", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (szPipeData.dwValue == 0x01)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::StartScanUsingService, FilePath: [%s]", csFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetProdutVersion
*  Description    :	Function which reads product version from registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 22 Nov 2017
**********************************************************************************************************/
CString CFilePropSheetExt::GetProdutVersion()
{
	try
	{
		TCHAR	szValueAppVersion[MAX_PATH] = { 0 };
		DWORD	dwSizeAppVersion = sizeof(szValueAppVersion);

		CITinRegWrapper objReg;
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csProdRegKey.GetBuffer(), L"AppVersion", szValueAppVersion, dwSizeAppVersion) == 0x00)
		{
			return CString(szValueAppVersion);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::GetProdutVersion", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}

/**********************************************************************************************************
*  Function Name  :	GetDatabaseVersion
*  Description    :	Function which reads DB version from registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 22 Nov 2017
**********************************************************************************************************/
CString CFilePropSheetExt::GetDatabaseVersion()
{
	try
	{
		TCHAR	szValueDBVersion[MAX_PATH] = { 0 };
		DWORD	dwSizeDBVersion = sizeof(szValueDBVersion);

		CITinRegWrapper objReg;
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csProdRegKey.GetBuffer(), L"DataBaseVersion", szValueDBVersion, dwSizeDBVersion) == 0x00)
		{
			return CString(szValueDBVersion);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::GetDatabaseVersion", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}

/**********************************************************************************************************
*  Function Name  :	GetScanEngineVersion
*  Description    :	Function which reads Scan Engine version from registry.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 22 Nov 2017
**********************************************************************************************************/
CString CFilePropSheetExt::GetScanEngineVersion()
{
	try
	{
		TCHAR	szValueScanVersion[MAX_PATH] = { 0 };
		DWORD	dwSizeScanVersion = sizeof(szValueScanVersion);

		CITinRegWrapper objReg;
		if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, m_csProdRegKey.GetBuffer(), L"ScanEngineVersion", szValueScanVersion, dwSizeScanVersion) == 0x00)
		{
			return CString(szValueScanVersion);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::GetScanEngineVersion", 0, 0, true, SECONDLEVEL);
	}
	return _T("");
}

/**********************************************************************************************************
*  Function Name  :	GetShortFilePath
*  Description    :	Function which return short file path
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 22 Nov 2017
**********************************************************************************************************/
CString CFilePropSheetExt::GetShortFilePath(CString csFilePath)
{
	CString csReturn = EMPTY_STRING;
	try
	{
		DWORD dwLegth = csFilePath.GetLength();
		if (dwLegth <= MAX_ALLOWED_PATH)
			return csFilePath;
		
		csReturn = csFilePath.Left(MAX_ALLOWED_PATH / 2);
		csReturn += _T("~");
		csReturn += csFilePath.Right((MAX_ALLOWED_PATH / 2) - 0x01);
		return csReturn;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::GetShortFilePath, FilePath: [%s]", csFilePath, 0, true, SECONDLEVEL);
	}
	return csReturn;
}

/**********************************************************************************************************
*  Function Name  :	LoadBitmapResource
*  Description    :	Function loads bitmap resource
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 22 Nov 2017
**********************************************************************************************************/
HBITMAP CFilePropSheetExt::LoadBitmapResource(DWORD dwBitmapID)
{
	__try
	{
		HBITMAP hBitmap = (HBITMAP)::LoadImage(_AtlBaseModule.m_hInst,
			MAKEINTRESOURCE(dwBitmapID), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);

		return hBitmap;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::LoadBitmapResource", 0, 0, true, SECONDLEVEL);
	}
	return NULL;
}

/**********************************************************************************************************
*  Function Name  :	MakeBold
*  Description    :	Function which makes BOLD font
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  : 
*  Date           : 22 Nov 2017
**********************************************************************************************************/
void CFilePropSheetExt::MakeBold(HWND hwnd)
{
	__try
	{
		if (!hwnd)
			return;

		HFONT hFontB, hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		LOGFONT lf;
		GetObject(hFont, sizeof(LOGFONT), &lf);
		lf.lfWeight = FW_BOLD;
		hFontB = CreateFontIndirect(&lf);
		SendMessage(hwnd, WM_SETFONT, (int)hFontB, 1);

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CFilePropSheetExt::MakeBold", 0, 0, true, SECONDLEVEL);
	}
}