/********************************************************************
	created:	2008/07/22
	created:	22:7:2008   10:25
	filename: 	SelectDialog.cpp
	file base:	SelectDialog
	file ext:	cpp
	author:		Hojjat Bohlooli - software@tarhafarinin.ir

	purpose:
*********************************************************************/
#include "stdafx.h"
#include <WinUser.h>
#include "WardWizUI.h"
#include ".\SelectDialog.h"

#pragma warning( push )
#pragma warning( disable : 4311 4312 )
// CSelectDialog
CString CSelectDialog::m_strCurrendDirectory;
CStringArray CSelectDialog::m_SelectedItemList;
WNDPROC CSelectDialog::m_wndProc = NULL;
CString ExpandShortcut(CString& csFilename);
IMPLEMENT_DYNAMIC(CSelectDialog, CFileDialog)

/***********************************************************************************************
*  Function Name  : CSelectDialog
*  Description    : Constructor
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CSelectDialog::CSelectDialog(BOOL bOpenFileDialog,
LPCTSTR lpszDefExt,
LPCTSTR lpszFileName,
DWORD dwFlags,
LPCTSTR lpszFilter,
CWnd* pParentWnd)
:CFileDialog(
bOpenFileDialog,
lpszDefExt,
lpszFileName,
dwFlags | OFN_NONETWORKBUTTON | OFN_EXPLORER | OFN_HIDEREADONLY & (~OFN_SHOWHELP),
lpszFilter,
pParentWnd,
0, NULL)
{
	dwFlags |= (OFN_NONETWORKBUTTON | OFN_EXPLORER | OFN_HIDEREADONLY & (~OFN_SHOWHELP));
};

/***********************************************************************************************
*  Function Name  : ~CSelectDialog
*  Description    : Destructor
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CSelectDialog::~CSelectDialog()
{
};

BEGIN_MESSAGE_MAP(CSelectDialog, CFileDialog)
END_MESSAGE_MAP()
/***********************************************************************************************
*  Function Name  : OnFileNameOK
*  Description    : CSelectDialog message handlers
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
BOOL CSelectDialog::OnFileNameOK()
{
	try
	{
		if (CFileDialog* pDlg = (CFileDialog*)CWnd::FromHandle(GetParent()->m_hWnd))
		{
			CWnd* pWnd = pDlg->GetDlgItem(lst2);	//getting list
			if (pWnd == NULL)
				return FALSE;

			m_SelectedItemList.RemoveAll();			// emptying list

			CListCtrl* wndLst1 = (CListCtrl*)(pWnd->GetDlgItem(1));

			int nSelected = wndLst1->GetSelectedCount();
			if (!nSelected)		// nothing selected -- don't retrieve list
				return FALSE;
			CString strItemText, strDirectory = m_strCurrendDirectory;
			if (strDirectory.Right(1) != _T("\\"))
				strDirectory += _T("\\");

			CString fileslist = _T("");
			pDlg->SendMessage(CDM_GETFILEPATH, (WPARAM)MAX_PATH,
				(LPARAM)fileslist.GetBuffer(MAX_PATH));
			fileslist.ReleaseBuffer();

			strItemText = strDirectory + fileslist;
			if (nSelected == 1 && fileslist != _T(""))
			{
				//m_SelectedItemList.Add(strItemText);
				//CString strPath = ExpandShortcut(fileslist);
				if (PathFileExists(fileslist))
				{
					m_SelectedItemList.Add(fileslist);
				}
				if (PathFileExists(strDirectory + fileslist))
				{
					m_SelectedItemList.Add(strDirectory + fileslist);
				}
				return CFileDialog::OnFileNameOK();
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSelectDialog::OnFileNameOK()", 0, 0, true, SECONDLEVEL);;
	}
	::MessageBeep(MB_ICONQUESTION);
	return 1; //don't let the dialog to close
};

/***********************************************************************************************
*  Function Name  : OnFolderChange
*  Description    : Called whenever the folder is changed
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
void CSelectDialog::OnFolderChange()
{
	try
	{
		m_strCurrendDirectory = GetFolderPath();
		CFileDialog::OnFolderChange();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSelectDialog::OnFolderChange()", 0, 0, true, SECONDLEVEL);;
	}
};

/***********************************************************************************************
*  Function Name  : OnInitDone
*  Description    : Used to Draw the UI for Browse window
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
void CSelectDialog::OnInitDone()
{
	try
	{
		m_strCurrendDirectory = GetFolderPath();
		CWnd* pFD = GetParent();

		HideControl(edt1);
		HideControl(cmb1);
		HideControl(stc2);
		//Issue: Custom Scan Browse window controls (edit box and text) set to hidden Resolved By: Nitin K Date:24th April 2015
		HideControl(cmb13);
		HideControl(stc3);

		CRect rectCancel; pFD->GetDlgItem(IDCANCEL)->GetWindowRect(&rectCancel);
		pFD->ScreenToClient(&rectCancel);

		CRect rectOK; pFD->GetDlgItem(IDOK)->GetWindowRect(&rectOK);
		pFD->ScreenToClient(&rectOK);
		pFD->GetDlgItem(IDOK)->SetWindowPos(0, rectCancel.left - rectOK.Width() - 5, rectCancel.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		/*CRect rectList2; pFD->GetDlgItem(lst1)->GetWindowRect(&rectList2);
		pFD->ScreenToClient(&rectList2);
		pFD->GetDlgItem(lst1)->SetWindowPos(0, 0, 0, rectList2.Width(), abs(rectList2.top - (rectCancel.top - 5)), SWP_NOMOVE | SWP_NOZORDER);*/

		CRect rectStatic; pFD->GetDlgItem(stc3)->GetWindowRect(&rectStatic);
		pFD->ScreenToClient(&rectStatic);
		pFD->GetDlgItem(stc3)->SetWindowPos(0, rectCancel.left - 375, rectCancel.top + 5, rectStatic.Width(), rectStatic.Height(), SWP_NOZORDER);

		CRect rectEdit1; pFD->GetDlgItem(cmb13)->GetWindowRect(&rectEdit1);
		pFD->ScreenToClient(&rectEdit1);
		pFD->GetDlgItem(cmb13)->SetWindowPos(0, rectCancel.left - 320, rectCancel.top, rectEdit1.Width() - 15, rectEdit1.Height(), SWP_NOZORDER);

		//Issue: Custom Scan Browse window controls (edit box and text) set to hidden Resolved By: Nitin K Date:24th April 2015
		//SetControlText(stc3, _T("Item name:"));
		SetControlText(IDOK, _T("Select"));

		m_wndProc = (WNDPROC)::SetWindowLongPtr(pFD->m_hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProcNew);
		pFD->CenterWindow();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSelectDialog::OnInitDone()", 0, 0, true, SECONDLEVEL);;
	}
};

/***********************************************************************************************
*  Function Name  : WindowProcNew
*  Description    : Called when user makes any selection and clicks Select button
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
LRESULT CALLBACK CSelectDialog::WindowProcNew(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return WindowProcNewSEH(hwnd, message, wParam, lParam);
	//return CallWindowProc(m_wndProc, hwnd, message, wParam, lParam);
}

/***********************************************************************************************
*  Function Name  : ExpandShortcut
*  Description    : It expands the given shortcut
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CString ExpandShortcut(CString& csFilename)
{
	CString csExpandedFile = NULL;

	try
	{
		USES_CONVERSION;		// For T2COLE() below

		//
		// Make sure we have a path
		//
		if (csFilename.IsEmpty())
		{
			ASSERT(FALSE);
			return csExpandedFile;
		}

		//
		// Get a pointer to the IShellLink interface
		//
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

LRESULT CALLBACK CSelectDialog::WindowProcNewSEH(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		if (message == WM_COMMAND)
		{
			if (HIWORD(wParam) == BN_DOUBLECLICKED)
			{
				return false;
			}
			if (HIWORD(wParam) == BN_CLICKED)
			{
				
				if (LOWORD(wParam) == IDOK)
				{
					if (CFileDialog* pDlg = (CFileDialog*)CWnd::FromHandle(hwnd))
					{
						m_SelectedItemList.RemoveAll();			// emptying list
						CWnd* pWnd = pDlg->GetDlgItem(lst2);	//getting list
						if (pWnd == NULL)
							return FALSE;

						CListCtrl* wndLst1 = (CListCtrl*)(pWnd->GetDlgItem(1));
						if (wndLst1 == NULL)
							return FALSE;

						int nSelected = wndLst1->GetSelectedCount();
						if (!nSelected)		// nothing selected -- don't retrieve list
							return FALSE;

						CString strItemText, strDirectory = m_strCurrendDirectory;
						if (strDirectory.Right(1) != _T("\\"))
							strDirectory += _T("\\");

						int nItem = wndLst1->GetNextItem(-1, LVNI_SELECTED);
						CString fileslist = _T("");
						pDlg->SendMessage(CDM_GETFILEPATH, (WPARAM)999,
							(LPARAM)fileslist.GetBuffer(999));
						fileslist.ReleaseBuffer();
						if ((m_strCurrendDirectory == L"") && (fileslist == L""))
							return false;
						//Issue: Custom Scan selection not happening for user\Desktop folder Resolved By: Nitin K Date:24th April 2015

						//Add directory names to list
						CStringArray csBlockedDirArr;// = {};
						csBlockedDirArr.Add(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_DIAG_EXCLUDE_COMP"));
						csBlockedDirArr.Add(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_DIAG_EXCLUDE_NETWORK"));
						csBlockedDirArr.Add(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_DIAG_EXCLUDE_BIN"));
						csBlockedDirArr.Add(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_DIAG_EXCLUDE_LIB"));
						while ((nSelected--) > 0)
						{
							strItemText = wndLst1->GetItemText(nItem, 0);
							//Issue: Custom Scan selection not happening for user\Desktop folder Resolved By: Nitin K Date:24th April 2015
							for (int i = 0; i < csBlockedDirArr.GetCount(); i++)
							{
								if (strItemText == csBlockedDirArr.GetAt(i))
								{
									//issue 1209 Multiple message box Neha Gharge
									//theApp.m_bIsPopUpDisplayed = true;
									pDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_DIAG_INVALID_SELECTION "), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
									//theApp.m_bIsPopUpDisplayed = false;
									return false;
								}
							}
							strItemText = strDirectory + strItemText;

							DWORD attr = GetFileAttributes(strItemText);
							//Issue: Custom Scan selection not happening for user\Desktop folder Resolved By: Nitin K Date:24th April 2015
							if ((attr != 0xFFFFFFFF) && (attr & FILE_ATTRIBUTE_DIRECTORY))
							{
								m_SelectedItemList.Add(strItemText);
								pDlg->EndDialog(IDOK);
								return NULL;
							}
							nItem = wndLst1->GetNextItem(nItem, LVNI_SELECTED);
						}
						//////////////////   Add FILE names to list
						strItemText = _T("");
						nSelected = wndLst1->GetSelectedCount();
						if (nSelected > m_SelectedItemList.GetCount())
						{
							int MoreThanOnFile = fileslist.Find(_T("\""));
							CString str = fileslist.TrimLeft(MoreThanOnFile);
							if (MoreThanOnFile != -1)
							{
								//for (int i = 0; i < fileslist.GetLength(); i++)
								//	if (fileslist[i] != '\"')
								//	{
								//		strItemText.AppendFormat(_T("%c"), fileslist[i]);
								//		if (fileslist[i - 1] == '\"' && fileslist[i] == ' ')
								//			strItemText.Delete(strItemText.GetLength() - 1);
								//	}
								//	else if (!strItemText.IsEmpty())
								//	{
								//		//m_SelectedItemList.Add(( strItemText));
								//		if (PathFileExists(strDirectory + strItemText))
								//		{
								//			m_SelectedItemList.Add((strDirectory + strItemText));
								//		}
								//		strItemText.Empty();
								//	}
							}
							else
							{
								//Issue: Not able to select the Drives in browse window
								//Resolved by Nitin K.
								//CString strPath = ExpandShortcut( fileslist);
								int ilength = fileslist.GetLength();
								int iFindChar = fileslist.Find('\\', --ilength);
								if (iFindChar == ilength)
								{
									fileslist.SetAt(ilength, '\0');
								}
								if (PathFileExists(fileslist))
								{
									m_SelectedItemList.Add(fileslist);
								}
								else //ISsue no 543 : Issue with Custom scan in selecting DVD drive / floppy disk.
								{
									//issue 1209 Multiple message box Neha Gharge
									//theApp.m_bIsPopUpDisplayed = true;
									pDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_SELECT_DIAG_PATH_DOESNOT_EXIST"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
									//theApp.m_bIsPopUpDisplayed = false;
								}
								if (PathFileExists(strDirectory + fileslist))
								{
									m_SelectedItemList.Add(strDirectory + fileslist);
								}
								//m_SelectedItemList.Add(strDirectory + fileslist);
								/*if (fileslist.Compare(L""))
								{
								if (PathFileExists(fileslist))
								{
								m_SelectedItemList.Add( fileslist);
								}
								else if (PathFileExists(strDirectory + fileslist))
								{
								m_SelectedItemList.Add(strDirectory + fileslist);
								}
								else
								{
								AddLogEntry(L"### Exception in CSelectDialog::No valid selection made", 0, 0, true, SECONDLEVEL);
								}
								}*/
							}
						}
						pDlg->EndDialog(IDOK);
						return NULL;
					} // if IDOK
				}
			} // if BN_CLICKED
		}// if WM_COMMAND
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSelectDialog::WindowProcNew()", 0, 0, true, SECONDLEVEL);
	}
	return CallWindowProc(m_wndProc, hwnd, message, wParam, lParam);
}
#pragma warning( pop )