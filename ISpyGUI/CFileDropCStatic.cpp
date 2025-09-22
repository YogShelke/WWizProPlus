/////////////////////////////////////////////////////////////////////////////
//
//	CFileFileDropListCtrl - Enhanced CListCtrl that accepts and
//							filters dropped files/folders.
//
//	Jan 2000, Stuart Carter, stuart.carter@hotmail.com
//
//	You're free to use, modify and distribute this code
//	as long as credit is given...
//
//		Thanks to:
//		Handling of droppped files modified from:
//			CDropEdit, 1997 Chris Losinger
//			http://www.codeguru.com/editctrl/filedragedit.shtml
//
//		Shortcut expansion code modified from :
//			CShortcut, 1996 Rob Warner
//
//
//	History:
//
//	Version	Date		Author				Description
//	-------	----------	-------------------	--------------------------------
//	1.0		20-01-2000	Stuart Carter		Initial
//

#include "stdafx.h"
#include "CFileDropCStatic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <afxdisp.h>	// OLE stuff
#include <shlwapi.h>	// Shell functions (PathFindExtension() in this case)
#include <afxpriv.h>	// ANSI to/from Unicode conversion macros
#include "ISpyGUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************************************************************
*  Function Name  : CFileDropCStatic
*  Description    : Constructor
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CFileDropCStatic::CFileDropCStatic(CListCtrl *m_List)
{
	try
	{
		// Default drop mode
		m_dropMode.iMask = DL_ACCEPT_FILES | DL_ACCEPT_FOLDERS;
		m_dropMode.pfnCallback = NULL;

		// Initialize OLE libraries
		m_bMustUninitOLE = FALSE;
		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		if (!pState->m_bNeedTerm) // TRUE if OleUninitialize needs to be called
		{
			HRESULT hr = ::OleInitialize(NULL);
			if (FAILED(hr))
				// Or something of your choosing...
				AfxMessageBox(_T("OLE initialization failed.\n\nMake sure that the OLE libraries are the correct version."));
			else
				m_bMustUninitOLE = TRUE;
		}
		//Added by Nitin K. 18th March 2015
		m_ClistControlObj = m_List;
		//Issue: Added Messagebox for Maximum limit reached for List control 
		//Added by: Nitin K. 28th March 2015
		m_bMaxLimitReached = FALSE;
		m_bIsNetworkPAth = FALSE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::CFileDropCStatic(CListCtrl *m_List)", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : CFileDropCStatic
*  Description    : Constructor (Default)
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CFileDropCStatic::CFileDropCStatic()
{
	try
	{
		// Default drop mode
		m_dropMode.iMask = DL_ACCEPT_FILES | DL_ACCEPT_FOLDERS;
		m_dropMode.pfnCallback = NULL;

		// Initialize OLE libraries
		m_bMustUninitOLE = FALSE;
		_AFX_THREAD_STATE* pState = AfxGetThreadState();
		if (pState != NULL)
		{
			if (!pState->m_bNeedTerm) // TRUE if OleUninitialize needs to be called
			{
				HRESULT hr = ::OleInitialize(NULL);
				if (FAILED(hr))
				{
					// Or something of your choosing...
					//AfxMessageBox(_T("OLE initialization failed.\n\nMake sure that the OLE libraries are the correct version."));
					AddLogEntry(L"### OLE initialization failed.\n\nMake sure that the OLE libraries are the correct version in CFileDropCStatic::CFileDropCStatic", 0, 0, true, SECONDLEVEL);;
				}
				else
				{
					m_bMustUninitOLE = TRUE;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::CFileDropCStatic()", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : ~CFileDropCStatic
*  Description    : Destructor
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CFileDropCStatic::~CFileDropCStatic()
{
	try
	{
		if (m_bMustUninitOLE)
		{
			::OleUninitialize();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::~CFileDropCStatic()", 0, 0, true, SECONDLEVEL);;
	}
}

BEGIN_MESSAGE_MAP(CFileDropCStatic, CStatic)
	//{{AFX_MSG_MAP(CFileDropListCtrl)
	ON_WM_DROPFILES()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***********************************************************************************************
*  Function Name  : OnCreate
*  Description    : This function is called to Register ourselves as a drop target for files
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
int CFileDropCStatic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try
	{
		if (CStatic::OnCreate(lpCreateStruct) == -1)
			return -1;

		// Register ourselves as a drop target for files
		DragAcceptFiles(TRUE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::OnCreate()", 0, 0, true, SECONDLEVEL);;
	}
	return 0;
}

/***********************************************************************************************
*  Function Name  : SetDropMode
*  Description    : Specify how the control will react to dropped files/folders.
*  Return value	  :
					FALSE:	the structure was not populated correctly,
							the default settings will be used.
					TRUE:	changes to the drop mode accepted
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
BOOL CFileDropCStatic::SetDropMode(const CFileDropCStatic::DROPLISTMODE& dropMode)
{
	try
	{
		// If they want to use a callback, ensure they also
		// specified the address of a function...
		if (dropMode.iMask & DL_USE_CALLBACK && dropMode.pfnCallback == NULL)
		{
			// Must specify a function if using DL_USE_CALLBACK flag
			ASSERT(FALSE);
			return FALSE;
		}
		m_dropMode = dropMode;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::SetDropMode()", 0, 0, true, SECONDLEVEL);;
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : OnDropFiles
*  Description    : Handle the WM_DROPFILES message
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
void CFileDropCStatic::OnDropFiles(HDROP dropInfo)
{
	try
	{
		// Get the number of pathnames (files or folders) dropped
		UINT nNumFilesDropped = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, 0);

		// Iterate through the pathnames and process each one
		TCHAR szFilename[MAX_PATH + 1] = { 0 };
		CString csPathname;
		CString csExpandedFilename;

		for (UINT nFile = 0; nFile < nNumFilesDropped; nFile++)
		{
			// Get the pathname
			DragQueryFile(dropInfo, nFile, szFilename, MAX_PATH + 1);

			// It might be shortcut, so try and expand it
			csPathname = szFilename;
			csExpandedFilename = ExpandShortcut(csPathname);
			if (!csExpandedFilename.IsEmpty())
			{
				csPathname = csExpandedFilename;
			}

			// Now see if its something we allow to be dropped
			UINT iPathType = 0;
			if (ValidatePathname(csPathname, iPathType))
			{
				// By default, we insert the filename into the list
				// ourselves, but if our parent wants to do something flashy
				// with it (maybe get the filesize and insert that as an extra
				// column), they will have installed a callback for each
				// droppped item
				if (m_dropMode.iMask & DL_USE_CALLBACK)
				{
					// Let them know about this list control and the item
					// droppped onto it
					if (m_dropMode.pfnCallback)
						m_dropMode.pfnCallback(this, csPathname, iPathType);
				}
				else
				{
					int ilength = csPathname.GetLength();
					int iFindChar = csPathname.Find('\\', --ilength);
					if (iFindChar>0)
					{
						csPathname.SetAt(ilength, '\0');
					}
					InsertPathname(csPathname);
				}
			}
			//Issue: Added Messagebox for Maximum limit reached for List control 
			//Added by: Nitin K. 28th March 2015
			if (m_bMaxLimitReached)
			{
				theApp.m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MAX_LIMIT_FOR_CUSTOM_SCAN"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
				theApp.m_bIsPopUpDisplayed = false;
				break;
			}
		}
		if (m_bIsNetworkPAth)
		{
			theApp.m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NETWORK_PATH_NOT_ALLOWED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
			theApp.m_bIsPopUpDisplayed = false;
		}
		m_bMaxLimitReached = FALSE;
		m_bIsNetworkPAth = FALSE;
		// Free the dropped-file info that was allocated by windows
		DragFinish(dropInfo);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::OnDropFiles()", 0, 0, true, SECONDLEVEL);;
	}
}

/***********************************************************************************************
*  Function Name  : ExpandShortcut
*  Description    : Uses IShellLink to expand a shortcut.
*  Return value	  : the expanded filename, or "" on error or if filename
					wasn't a shortcut
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CString CFileDropCStatic::ExpandShortcut(CString& csFilename) const
{
	CString csExpandedFile = NULL;

	try
	{
		USES_CONVERSION;		// For T2COLE() below

		// Make sure we have a path
		if (csFilename.IsEmpty())
		{
			ASSERT(FALSE);
			return csExpandedFile;
		}

		// Get a pointer to the IShellLink interface
		HRESULT hr;
		IShellLink* pIShellLink;

		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&pIShellLink);

		if (SUCCEEDED(hr))
		{
			// Get a pointer to the persist file interface
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);

			if (SUCCEEDED(hr))
			{
				// Load the shortcut and resolve the path
				// IPersistFile::Load() expects a UNICODE string
				// so we're using the T2COLE macro for the conversion
				// For more info, check out MFC Technical note TN059
				// (these macros are also supported in ATL and are
				// so much better than the ::MultiByteToWideChar() family)
				hr = pIPersistFile->Load(T2COLE(csFilename), STGM_READ);

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csExpandedFile.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_UNCPRIORITY);

					csExpandedFile.ReleaseBuffer(-1);
				}
				pIPersistFile->Release();
			}
			pIShellLink->Release();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::ExpandShortcut()", 0, 0, true, SECONDLEVEL);;
	}
	return csExpandedFile;
}


//////////////////////////////////////////////////////////////////
//
//	ValidatePathname()
//
//	Checks if a pathname is valid based on these options set:
//		Allow directories to be dropped
//		Allow files to be dropped
//		Only allow files with a certain extension to be dropped
//
//	
//
//		If successful, iPathType specifies the type of path
//		validated - either a file or a folder.
/***********************************************************************************************
*  Function Name  : ValidatePathname
*  Description    : Checks if a pathname is valid based on these options set:
						Allow directories to be dropped
						Allow files to be dropped
						Only allow files with a certain extension to be dropped
*  Return value	  :
					TRUE:	the pathname is suitable for selection, or
					FALSE:	the pathname failed the checks.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
BOOL CFileDropCStatic::ValidatePathname(const CString& csPathname, UINT& iPathType) const
{
	BOOL bValid = FALSE;

	try
	{
		//
		// Get some info about that path so we can filter out dirs
		// and files if need be
		//

		//Issue : 00000867 In custom scan if drag-n-drop more than 3GB file then no action is taking place.
		struct _stat64 buf;
		int result = _tstat64(csPathname, &buf);
		DWORD dwErr =GetLastError();
		if (result == 0)
		{
			//
			// Do we have a directory? (if we want dirs)
			//
			if ((m_dropMode.iMask & DL_ACCEPT_FOLDERS) &&
				((buf.st_mode & _S_IFDIR) == _S_IFDIR))
			{
				bValid = TRUE;
				iPathType = DL_FOLDER_TYPE;
			}
			else if ((m_dropMode.iMask & DL_ACCEPT_FILES) &&
				((buf.st_mode & _S_IFREG) == _S_IFREG))
			{
				// 
				// We've got a file and files are allowed.
				//
				iPathType = DL_FILE_TYPE;

				//
				// Now if we are filtering out specific types,
				// check the file extension.
				//
				if (m_dropMode.iMask & DL_FILTER_EXTENSION)
				{
					LPTSTR pszFileExt = PathFindExtension(csPathname);
					if (CString(pszFileExt).CompareNoCase(m_dropMode.csFileExt) == 0)
					{
						bValid = TRUE;
					}
				}
				else
				{
					bValid = TRUE;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::ValidatePathname()", 0, 0, true, SECONDLEVEL);;
	}
	return bValid;
}

/***********************************************************************************************
*  Function Name  : InsertPathname
*  Description    : This is used to insert a dropped item when a callback function hasn't been specified.
					It also checks if duplicate files are allowed to be inserted
					and does the necessary.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
int CFileDropCStatic::InsertPathname(const CString& csFilename)
{
	try
	{
		if (m_ClistControlObj == NULL)
		{
			return -1;
		}

		if (!(m_dropMode.iMask & DL_ALLOW_DUPLICATES))
		{
			//
			// We don't allow duplicate pathnames, so
			// see if this one is already in the list.
			//
			LVFINDINFO lvInfo;
			lvInfo.flags = LVFI_STRING;
			lvInfo.psz = csFilename;


			if (m_ClistControlObj->FindItem(&lvInfo, -1) != -1)
				return -1;
			if (PathIsNetworkPath(csFilename))
			{
				m_bIsNetworkPAth = TRUE;
				return -1;
			}
		}
		//Issue: Added Messagebox for Maximum limit reached for List control 
		//Added by: Nitin K. 28th March 2015
		CString csScanPath;
		int iCount = 0;
		int iCListEntriesLength = 0;
		for (iCount = 0; iCount < m_ClistControlObj->GetItemCount(); iCount++)
		{
			csScanPath = m_ClistControlObj->GetItemText(iCount, 0);
			iCListEntriesLength += csScanPath.GetLength();	
		}
		iCListEntriesLength += iCount;
		if ((1000 - iCListEntriesLength) > csFilename.GetLength())
		{
			m_ClistControlObj->InsertItem(0, csFilename);
			//Issue: Check box Enabled for Custom Scan List box Resolved By: Nitin K Date:24th April 2015
			m_ClistControlObj->SetCheck(0, TRUE);
		}
		else
		{
			m_bMaxLimitReached = TRUE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CFileDropCStatic::InsertPathname()", 0, 0, true, SECONDLEVEL);;
	}
	return 0;// InsertItem(0, csFilename);
}
