/**********************************************************************************************************
Program Name          : PatchMerger.cpp
Description           : This class is used to merge the patches on XAMPP server.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 22 Mar 2016
Version No            : 1.13.1.10
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizALUpd.h"
#include "PatchMerger.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CPatchMerger, CDialog)

/***************************************************************************
Function Name  : CPatchMerger
Description    : cont'r
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
CPatchMerger::CPatchMerger(CWnd* pParent /*=NULL*/)
: CDialog(CPatchMerger::IDD, pParent)
{

	
}

/***************************************************************************
Function Name  : ~CPatchMerger
Description    : Dest'r
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
CPatchMerger::~CPatchMerger()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : DDX mechanism
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO		   :
****************************************************************************/
void CPatchMerger::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LEFT_FOLDER, m_editLeftFolder);
	DDX_Control(pDX, IDC_EDIT_RIGHT_FOLDER, m_editRightFolder);
	DDX_Control(pDX, IDC_BUTTON_LBROWSE, m_btnLeftBrowse);
	DDX_Control(pDX, IDC_BUTTON_RBROWSE, m_btnRightBrowse);
	DDX_Control(pDX, IDC_BUTTON_COMPARE, m_btnCompare);
	DDX_Control(pDX, IDC_BUTTON_MERGE, m_btnMerge);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatus);
	DDX_Control(pDX, IDC_LIST_BASIC, m_lstBasic);
	DDX_Control(pDX, IDC_LIST_ESSENTIAL, m_lstEssential);
	DDX_Control(pDX, IDC_LIST_PRO, m_lstPro);
	DDX_Control(pDX, IDC_CHECK_BSELECTALL, m_chkBasic);
	DDX_Control(pDX, IDC_CHECK_ESELECTALL, m_chkEssential);
	DDX_Control(pDX, IDC_CHECK_PSELECTALL, m_chkPro);
	DDX_Control(pDX, IDC_STATIC_BASICCOUNT, m_stBasicCount);
	DDX_Control(pDX, IDC_STATIC_ESSENTIALCOUNT, m_stEssentialCount);
	DDX_Control(pDX, IDC_STATIC_ELITECOUNT, m_stEliteCount);
	DDX_Control(pDX, IDC_STATIC_ESSPLUSCOUNT, m_stEssPlusCount);
	DDX_Control(pDX, IDC_STATIC_PROCOUNT, m_stProCount);
	DDX_Control(pDX, IDC_LIST_PRO2, m_lstElite);
	DDX_Control(pDX, IDC_LIST_PRO3, m_lstEssPlus);
	DDX_Control(pDX, IDC_LIST_PRO4, m_lstEliteServer);
	DDX_Control(pDX, IDC_STATIC_ELITECOUNT2, m_stEliteServer);
	DDX_Control(pDX, IDC_CHECK_ELITESELECTALL, m_chkEliteClient);
	DDX_Control(pDX, IDC_CHECK_ELITESERVERSELECTALL, m_chkEliteServer);
	DDX_Control(pDX, IDC_CHECK_ESSPLUSSELECTALL, m_chkEssentialPlus);
}


BEGIN_MESSAGE_MAP(CPatchMerger, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE, &CPatchMerger::OnBnClickedButtonCompare)
	ON_BN_CLICKED(IDC_BUTTON_LBROWSE, &CPatchMerger::OnBnClickedButtonLbrowse)
	ON_BN_CLICKED(IDC_BUTTON_RBROWSE, &CPatchMerger::OnBnClickedButtonRbrowse)
	ON_BN_CLICKED(IDC_CHECK_BSELECTALL, &CPatchMerger::OnBnClickedCheckBselectall)
	ON_BN_CLICKED(IDC_CHECK_ESELECTALL, &CPatchMerger::OnBnClickedCheckEselectall)
	ON_BN_CLICKED(IDC_CHECK_PSELECTALL, &CPatchMerger::OnBnClickedCheckPselectall)
	ON_BN_CLICKED(IDC_CHECK_ELITESELECTALL, &CPatchMerger::OnBnClickedCheckEliteClientselectall)
	ON_BN_CLICKED(IDC_CHECK_ELITESERVERSELECTALL, &CPatchMerger::OnBnClickedCheckEliteServerselectall)
	ON_BN_CLICKED(IDC_CHECK_ESSPLUSSELECTALL, &CPatchMerger::OnBnClickedCheckEssentialPlusselectall)
	ON_EN_CHANGE(IDC_EDIT_LEFT_FOLDER, &CPatchMerger::OnEnChangeEditLeftFolder)
	ON_EN_CHANGE(IDC_EDIT_RIGHT_FOLDER, &CPatchMerger::OnEnChangeEditRightFolder)
	ON_BN_CLICKED(IDC_BUTTON_MERGE, &CPatchMerger::OnBnClickedButtonMerge)
END_MESSAGE_MAP()

/***************************************************************************
Function Name  : OnInitDialog
Description    : Function which gets called from MFC framework
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO		   :
****************************************************************************/
BOOL CPatchMerger::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitializeVariables();

	CRect oRcRect;
	m_lstBasic.GetWindowRect(&oRcRect);

	int iWidth = oRcRect.Width() - 5;

	m_lstBasic.InsertColumn(0, L"WardWiz Basic", LVCFMT_LEFT, iWidth);
	m_lstBasic.InsertColumn(1, L"SectionName", LVCFMT_LEFT, 0);
	m_lstBasic.InsertColumn(2, L"KeyName", LVCFMT_LEFT, 0);

	m_lstEssential.InsertColumn(0, L"WardWiz Essential", LVCFMT_LEFT, iWidth);
	m_lstEssential.InsertColumn(1, L"SectionName", LVCFMT_LEFT, 0);
	m_lstEssential.InsertColumn(2, L"KeyName", LVCFMT_LEFT, 0);

	m_lstPro.InsertColumn(0, L"WardWiz Pro", LVCFMT_LEFT, iWidth);
	m_lstPro.InsertColumn(1, L"SectionName", LVCFMT_LEFT, 0);
	m_lstPro.InsertColumn(2, L"KeyName", LVCFMT_LEFT, 0);

	m_lstElite.InsertColumn(0, L"WardWiz Elite", LVCFMT_LEFT, iWidth);
	m_lstElite.InsertColumn(1, L"SectionName", LVCFMT_LEFT, 0);
	m_lstElite.InsertColumn(2, L"KeyName", LVCFMT_LEFT, 0);

	m_lstEliteServer.InsertColumn(0, L"WardWiz Elite Server", LVCFMT_LEFT, iWidth);
	m_lstEliteServer.InsertColumn(1, L"SectionName", LVCFMT_LEFT, 0);
	m_lstEliteServer.InsertColumn(2, L"KeyName", LVCFMT_LEFT, 0);

	m_lstEssPlus.InsertColumn(0, L"WardWiz Essential Plus", LVCFMT_LEFT, iWidth);
	m_lstEssPlus.InsertColumn(1, L"SectionName", LVCFMT_LEFT, 0);
	m_lstEssPlus.InsertColumn(2, L"KeyName", LVCFMT_LEFT, 0);

	ListView_SetExtendedListViewStyle(m_lstBasic.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	ListView_SetExtendedListViewStyle(m_lstEssential.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	ListView_SetExtendedListViewStyle(m_lstPro.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	ListView_SetExtendedListViewStyle(m_lstElite.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	ListView_SetExtendedListViewStyle(m_lstEliteServer.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	ListView_SetExtendedListViewStyle(m_lstEssPlus.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);

	SetStatus(L"Please select valid folders to compare");

	return TRUE;  // return TRUE unless you set the focus to a control
}

/***************************************************************************
Function Name  : InitializeVariables
Description    : Function to initializes required variables on application start
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO		   :
****************************************************************************/
void CPatchMerger::InitializeVariables()
{
	m_btnCompare.EnableWindow(FALSE);
	m_btnMerge.EnableWindow(FALSE);
}

/***************************************************************************
Function Name  : SetStatus
Description    : Function to set status on status edit control
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::SetStatus(CString csStatus)
{
	m_editStatus.SetWindowText(csStatus);
}

/***************************************************************************
Function Name  : OnBnClickedButtonLbrowse
Description    : Button click handler for left folder selection
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedButtonLbrowse()
{
	CString csLFolder = GetSelectedFolder();
	m_editLeftFolder.SetWindowText(csLFolder);

	CString csRFolder;
	m_editRightFolder.GetWindowText(csRFolder);

	if (csLFolder.Trim().GetLength() != 0 && csRFolder.Trim().GetLength() != 0)
	{
		m_btnCompare.EnableWindow(TRUE);
	}
}

/***************************************************************************
Function Name  : OnBnClickedButtonRbrowse
Description    : Button click handler for right folder selection
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedButtonRbrowse()
{
	CString csRFolder = GetSelectedFolder();
	m_editRightFolder.SetWindowText(csRFolder);

	CString csLFolder;
	m_editLeftFolder.GetWindowText(csLFolder);

	if (csLFolder.Trim().GetLength() != 0 && csRFolder.Trim().GetLength() != 0)
	{
		m_btnCompare.EnableWindow(TRUE);
	}
}

/***************************************************************************
Function Name  : GetSelectedFolder
Description    : Common functionality kept in separate function.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
CString CPatchMerger::GetSelectedFolder()
{
	CString csTemp;
	TCHAR *pszPath = new TCHAR[MAX_PATH];
	SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

	CString csMessage = L"Please select valid folder";
	BROWSEINFO        bi = { m_hWnd, NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0 };

	LPITEMIDLIST      pIdl = NULL;

	LPITEMIDLIST  pRoot = NULL;
	//SHGetFolderLocation(m_hWnd, CSIDL_DRIVES, 0, 0, &pRoot);
	bi.pidlRoot = pRoot;
	pIdl = SHBrowseForFolder(&bi);
	if (NULL != pIdl)
	{
		SHGetPathFromIDList(pIdl, pszPath);
		size_t iLen = wcslen(pszPath);
		if (iLen > 0)
		{
			csTemp = pszPath;
			if (csTemp.Right(1) == L"\\")
			{
				csTemp = csTemp.Left((int)iLen - 1);
			}
		}
	}
	delete[] pszPath;
	pszPath = NULL;

	return CString(csTemp);
}


/***************************************************************************
Function Name  : OnBnClickedButtonCompare
Description    : Button click handler to compare folders
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedButtonCompare()
{
	CString csLPath, csRPath;

	m_editRightFolder.GetWindowText(csRPath);
	m_editLeftFolder.GetWindowText(csLPath);

	m_lstBasic.DeleteAllItems();
	m_lstEssential.DeleteAllItems();
	m_lstPro.DeleteAllItems();
	m_lstElite.DeleteAllItems();
	m_lstEliteServer.DeleteAllItems();
	m_lstEssPlus.DeleteAllItems();

	if (!PathFileExists(csLPath) || !PathFileExists(csRPath))
	{
		SetStatus(L"Please select valid paths");
		return;
	}

	if (!CompareFolder(csLPath, csRPath))
	{
		SetStatus(L"Folders are iddentical");
		return;
	}

	SetStatus(L"Difference found in folder, please click merge button to merge Changes from Left changes to right");

	m_btnCompare.EnableWindow(FALSE);
	m_btnMerge.EnableWindow(TRUE);

	int iCount = m_lstBasic.GetItemCount();
	CString csFilesCount;
	csFilesCount.Format(L"Files Count: %d", iCount);
	m_stBasicCount.SetWindowText(csFilesCount);

	iCount = m_lstEssential.GetItemCount();
	csFilesCount.Format(L"Files Count: %d", iCount);
	m_stEssentialCount.SetWindowText(csFilesCount);

	iCount = m_lstPro.GetItemCount();
	csFilesCount.Format(L"Files Count: %d", iCount);
	m_stProCount.SetWindowText(csFilesCount);

	iCount = m_lstElite.GetItemCount();
	csFilesCount.Format(L"Files Count: %d", iCount);
	m_stEliteCount.SetWindowText(csFilesCount);

	iCount = m_lstEliteServer.GetItemCount();
	csFilesCount.Format(L"Files Count: %d", iCount);
	m_stEliteServer.SetWindowText(csFilesCount);

	iCount = m_lstEssPlus.GetItemCount();
	csFilesCount.Format(L"Files Count: %d", iCount);
	m_stEssPlusCount.SetWindowText(csFilesCount);
}

/***************************************************************************
Function Name  : CompareFolder
Description    : Function which accepts folder paths and comparing files using ini flies.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::CompareFolder(CString csLPath, CString csRPath)
{
	bool bReturn = false;

	CStringArray csFiles;
	csFiles.Add(L"WWizPatchB.ini");
	csFiles.Add(L"VibroPatchTS.ini");
	csFiles.Add(L"WWizPatchP.ini");
	csFiles.Add(L"WWizPatchT.ini");
	csFiles.Add(L"WWizPatchEPS.ini");
	csFiles.Add(L"VibroPatchAS.ini");

	for (DWORD dwCount = 0x00; dwCount < csFiles.GetCount(); dwCount++)
	{
		CString csFileName = csFiles[dwCount];
		if (CompareFiles(dwCount + 1, csLPath + L"\\" + csFileName, csRPath + L"\\" + csFileName))
		{
			bReturn = true;
		}
	}
	return bReturn;
}

/***************************************************************************
Function Name  : CompareFiles
Description    : Function which compares each file
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::CompareFiles(DWORD dwProdID, CString csLPath, CString csRPath)
{
	bool bReturn = false;

	CStringArray csSectionNames;
	csSectionNames.Add(L"ProductVersion");
	csSectionNames.Add(L"DatabaseVersion");
	csSectionNames.Add(L"DataEncVersion");
	csSectionNames.Add(L"Files_32");
	csSectionNames.Add(L"Files_64");
	csSectionNames.Add(L"Common");
	csSectionNames.Add(L"CommonDB");
	csSectionNames.Add(L"DllRegister");

	for (DWORD dwCount = 0x00; dwCount < csSectionNames.GetCount(); dwCount++)
	{
		if (CompareSections(dwProdID, csSectionNames[dwCount], csLPath, csRPath))
		{
			bReturn = true;
			//make addlog entry here
		}
	}

	return bReturn;
}

/***************************************************************************
Function Name  : CompareSections
Description    : Function to compare sections
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::CompareSections(DWORD dwProductID, CString csSectionName, CString csLPath, CString csRPath)
{
	bool bReturn = false;

	TCHAR	szValueName[256] = { 0 };
	TCHAR	szValueData[2048] = { 0 };


	//Special case need to handle separately
	CString csCountSection = csSectionName;
	if (csSectionName == L"Files_32")
		csCountSection = L"Count_32";
	else if (csSectionName == L"Files_64")
		csCountSection = L"Count_64";
	else if (csSectionName == L"CommonDB")
		csCountSection = L"CommonDB";

	if (csCountSection.Find(L"Version", 0) > 0)
	{
		CString csKey;
		if (csCountSection == L"ProductVersion")
		{
			csKey = L"ProductVer";
		}
		else if (csCountSection == L"DatabaseVersion")
		{
			csKey = L"DatabaseVer";
		}
		else if (csCountSection == L"DataEncVersion")
		{
			csKey = L"DataEncVer";
		}

		ZeroMemory(szValueName, sizeof(szValueName));
		GetPrivateProfileString(csSectionName, csKey, L"", szValueData, _countof(szValueData), csLPath);

		if (ISDifferenceWithSecondFile(csRPath, csSectionName.GetBuffer(), csKey.GetBuffer(), szValueData))
		{
			DWORD dwKeyID = 0;
			if (!InsertInList(dwProductID, csSectionName.GetBuffer(), szValueData, dwKeyID))
			{
				return false;
			}
			return true;
		}
		return false;
	}

	DWORD dwCount = GetPrivateProfileInt(csCountSection, L"Count", 0x00, csLPath);

	DWORD	iIndex = 0x01;
	for (; iIndex <= dwCount; iIndex++)
	{
		ZeroMemory(szValueName, sizeof(szValueName));
		swprintf_s(szValueName, _countof(szValueName), L"%lu", iIndex);
		GetPrivateProfileString(csSectionName, szValueName, L"", szValueData, _countof(szValueData), csLPath);

		TCHAR szExeName[MAX_PATH] = { 0 };
		TCHAR szZipSize[MAX_PATH] = { 0 };
		TCHAR szFullSize[MAX_PATH] = { 0 };
		TCHAR szFileHash[MAX_PATH] = { 0 };

		CString csData = szValueData;

		if (csSectionName != L"EnhancedFunctionality")
		{
			if (!ParseIniLine(szValueData, szExeName, szZipSize, szFullSize, szFileHash))
			{
				continue;
			}
		}

		DWORD dwIndexKey = 0x00;
		if (ISDifferenceWithSecondFile(csRPath, csSectionName.GetBuffer(), szExeName, szZipSize, szFullSize, szFileHash, dwIndexKey))
		{
			InsertInList(dwProductID, csSectionName.GetBuffer(), csData.GetBuffer(), dwIndexKey);
		}
	}

	//Check here any difference found
	if (m_lstBasic.GetItemCount() != 0 || m_lstEssential.GetItemCount() != 0 || m_lstPro.GetItemCount() != 0
		|| m_lstElite.GetItemCount() != 0 || m_lstEliteServer.GetItemCount() != 0 || m_lstEssPlus.GetItemCount() != 0)
	{
		bReturn = true;
	}

	return bReturn;
}

/***************************************************************************
Function Name  : ParseIniLine
Description    : Function which parse the line and extract required values from it.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::ParseIniLine(LPTSTR lpszIniLine, LPTSTR lpExeName, LPTSTR lpszZipSize, LPTSTR lpszFullSize, LPTSTR lpszFileHash)
{
	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;

	pToken = wcstok(lpszIniLine, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	TCHAR	szExeName[256] = { 0 };

	if (pToken)
		wcscpy(szExeName, pToken);

	_tcscpy(lpExeName, szExeName);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	TCHAR	szZipSize[256] = { 0 };
	DWORD	dwZipSize = 0x00;

	if (pToken)
	{
		wcscpy(szZipSize, pToken);
		swscanf(szZipSize, L"%lu", &dwZipSize);
	}

	_tcscpy(lpszZipSize, szZipSize);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	TCHAR	szFullSize[256] = { 0 };
	DWORD	dwFullSize = 0x00;

	if (pToken)
	{
		wcscpy(szFullSize, pToken);
		swscanf(szFullSize, L"%lu", &dwFullSize);
	}
	_tcscpy(lpszFullSize, szFullSize);

	pToken = wcstok(NULL, szToken);
	if (!pToken)
	{
		AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
		return false;
	}

	TCHAR	szFileHash[256] = { 0 };

	if (pToken)
		wcscpy(szFileHash, pToken);

	_tcscpy(lpszFileHash, szFileHash);

	return true;
}

/***************************************************************************
Function Name  : ISDifferenceWithSecondFile
Description    : Function which takes input value as exe name and check is difference found.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::ISDifferenceWithSecondFile(CString csFilePath, LPTSTR lpszIniLine, LPTSTR lpExeName, LPTSTR lpZipSize, LPTSTR lpFullSize, LPTSTR lpFileHash, DWORD &dwIndex)
{
	bool bReturn = false;

	if (!PathFileExists(csFilePath))
	{
		return bReturn;
	}

	TCHAR	szValueName[256] = { 0 };
	TCHAR	szValueData[2048] = { 0 };

	//Special case need to handle separately
	CString csCountSection = lpszIniLine;
	if ( _tcscmp(lpszIniLine, L"Files_32") == 0)
		csCountSection = L"Count_32";
	else if(_tcscmp(lpszIniLine, L"Files_64") == 0)
		csCountSection = L"Count_64";
	else if (_tcscmp(lpszIniLine, L"CommonDB") == 0)
		csCountSection = L"CommonDB";

	DWORD dwCount = GetPrivateProfileInt(csCountSection, L"Count", 0x00, csFilePath);

	DWORD	iIndex = 0x01;
	for (; iIndex <= dwCount; iIndex++)
	{
		ZeroMemory(szValueName, sizeof(szValueName));
		swprintf_s(szValueName, _countof(szValueName), L"%lu", iIndex);
		GetPrivateProfileString(lpszIniLine, szValueName, L"", szValueData, _countof(szValueData), csFilePath);

		TCHAR szExeName[MAX_PATH] = { 0 };
		TCHAR szZipSize[MAX_PATH] = { 0 };
		TCHAR szFullSize[MAX_PATH] = { 0 };
		TCHAR szFileHash[MAX_PATH] = { 0 };

		if (csCountSection != L"EnhancedFunctionality")
		{
			if (!ParseIniLine(szValueData, szExeName, szZipSize, szFullSize, szFileHash))
			{
				continue;
			}
		}

		if (_tcscmp(lpExeName, szExeName) == 0)
		{
			if (_tcscmp(lpZipSize, szZipSize) != 0)
			{
				dwIndex = iIndex;
				return true;
			}

			if (_tcscmp(lpFullSize, szFullSize) != 0)
			{
				dwIndex = iIndex;
				return true;
			}

			if (_tcscmp(lpFileHash, szFileHash) != 0)
			{
				dwIndex = iIndex;
				return true;
			}

			return false;
		}
	}

	CString csKey;
	if(GetNewKeyFromDestinationFile(csFilePath, csCountSection, csKey))
	{
		if (csKey.Trim().GetLength() == 0)
		{
			return bReturn;
		}

		iIndex = _wtoi(csKey);
		iIndex++;

		static CString csLastFilePath = L"";
		static CString csLastSection = L"";
		static DWORD dwLastIndex = 0;
		if (dwLastIndex == 0 || csLastFilePath != csFilePath || csLastSection != csCountSection)
		{
			dwLastIndex = iIndex;
		}

		csLastFilePath = csFilePath;
		csLastSection = csCountSection;

		//This is new entry not append this section and increase count.
		dwIndex = dwLastIndex++;

		bReturn = true;
	}

	return bReturn;
}

/***************************************************************************
Function Name  : InsertInList
Description    : function which inserts the record in proper list control
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::InsertInList(DWORD dwProductID, LPTSTR szSectionName, LPTSTR szData, DWORD &dwIndex)
{
	bool bReturn = false;

	CString csIndex;
	csIndex.Format(L"%d", dwIndex);

	int iItemCount = 0;
	switch (dwProductID)
	{
	case 1:
		iItemCount = m_lstBasic.GetItemCount();
		m_lstBasic.InsertItem(iItemCount, szData);
		m_lstBasic.SetItemText(iItemCount, 1, szSectionName);
		m_lstBasic.SetItemText(iItemCount, 2, csIndex);
		break;
	case 2:
		iItemCount = m_lstEssential.GetItemCount();
		m_lstEssential.InsertItem(iItemCount, szData);
		m_lstEssential.SetItemText(iItemCount, 1, szSectionName);
		m_lstEssential.SetItemText(iItemCount, 2, csIndex);
		break;
	case 3:
		iItemCount = m_lstPro.GetItemCount();
		m_lstPro.InsertItem(iItemCount, szData);
		m_lstPro.SetItemText(iItemCount, 1, szSectionName);
		m_lstPro.SetItemText(iItemCount, 2, csIndex);
		break;
	case 4:
		iItemCount = m_lstElite.GetItemCount();
		m_lstElite.InsertItem(iItemCount, szData);
		m_lstElite.SetItemText(iItemCount, 1, szSectionName);
		m_lstElite.SetItemText(iItemCount, 2, csIndex);
		break;
	case 5:
		iItemCount = m_lstEliteServer.GetItemCount();
		m_lstEliteServer.InsertItem(iItemCount, szData);
		m_lstEliteServer.SetItemText(iItemCount, 1, szSectionName);
		m_lstEliteServer.SetItemText(iItemCount, 2, csIndex);
		break;
	case 6:
		iItemCount = m_lstEssPlus.GetItemCount();
		m_lstEssPlus.InsertItem(iItemCount, szData);
		m_lstEssPlus.SetItemText(iItemCount, 1, szSectionName);
		m_lstEssPlus.SetItemText(iItemCount, 2, csIndex);
		break;
	default:
		break;
	}

	return bReturn;
}

/***************************************************************************
Function Name  : OnBnClickedCheckBselectall
Description    : Check box button click handler
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedCheckBselectall()
{
	SelectAll(m_lstBasic, m_chkBasic.GetCheck());
}

/***************************************************************************
Function Name  : OnBnClickedCheckEselectall
Description    : Check box button click handler
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedCheckEselectall()
{
	SelectAll(m_lstEssential, m_chkEssential.GetCheck());
}

/***************************************************************************
Function Name  : OnBnClickedCheckPselectall
Description    : Check box button click handler
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedCheckPselectall()
{
	SelectAll(m_lstPro, m_chkPro.GetCheck());
}
/***************************************************************************
Function Name  : OnBnClickedCheckPselectall
Description    : Check box button click handler
Author Name    : Tejas Tanaji Shinde
Date           : 10 May 2018
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedCheckEliteClientselectall()
{
	SelectAll(m_lstElite, m_chkEliteClient.GetCheck());
}
/***************************************************************************
Function Name  : OnBnClickedCheckPselectall
Description    : Check box button click handler
Author Name    : Tejas Tanaji Shinde
Date           : 10 May 2018
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedCheckEliteServerselectall()
{
	SelectAll(m_lstEliteServer, m_chkEliteServer.GetCheck());
}
/***************************************************************************
Function Name  : OnBnClickedCheckPselectall
Description    : Check box button click handler
Author Name    : Tejas Tanaji Shinde
Date           : 10 May 2018
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedCheckEssentialPlusselectall()
{
	SelectAll(m_lstEssPlus, m_chkEssentialPlus.GetCheck());
}
/***************************************************************************
Function Name  : SelectAll
Description    : common function which takes list control handler and selects all the entries present
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::SelectAll(CListCtrl &objlst, BOOL bSelect)
{
	for (int iIndex = 0; iIndex < objlst.GetItemCount(); iIndex++)
	{
		objlst.SetCheck(iIndex, bSelect);
	}
}

/***************************************************************************
Function Name  : OnEnChangeEditLeftFolder
Description    : MFC framework calls this function when there is change in left edit control
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnEnChangeEditLeftFolder()
{
	CheckValidPaths();
}

/***************************************************************************
Function Name  : OnEnChangeEditRightFolder
Description    : MFC framework calls this function when there is change in right edit control
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnEnChangeEditRightFolder()
{
	CheckValidPaths();
}

/***************************************************************************
Function Name  : CheckValidPaths
Description    : Function which checks for valid paths
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::CheckValidPaths()
{
	CString csLFolder;
	CString csRFolder;
	m_editLeftFolder.GetWindowText(csLFolder);
	m_editRightFolder.GetWindowText(csRFolder);

	m_btnCompare.EnableWindow(FALSE);
	if (csLFolder.Trim().GetLength() != 0 && csRFolder.Trim().GetLength() != 0)
	{
		if (PathFileExists(csLFolder) && PathFileExists(csRFolder))
		{
			m_btnCompare.EnableWindow(TRUE);
		}
	}
}

/***************************************************************************
Function Name  : OnBnClickedButtonMerge
Description    : Button click handler for merge button
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
void CPatchMerger::OnBnClickedButtonMerge()
{
	bool bItemBasic = IsItemsChecked(m_lstBasic);
	bool bItemEss = IsItemsChecked(m_lstEssential);
	bool bItemPro = IsItemsChecked(m_lstPro);
	bool bItemElite = IsItemsChecked(m_lstElite);
	bool bItemEliteServer = IsItemsChecked(m_lstEliteServer);
	bool bItemEssPlus = IsItemsChecked(m_lstEssPlus);

	if (!bItemBasic && !bItemEss && !bItemPro && !bItemElite && !bItemEliteServer &&!bItemEssPlus)
	{
		return;
		SetStatus(L"Please select items to merge");
	}

	//do here merging.
	CStringArray csFiles;
	csFiles.Add(L"WWizPatchB.ini");
	csFiles.Add(L"VibroPatchTS.ini");
	csFiles.Add(L"WWizPatchP.ini");
	csFiles.Add(L"WWizPatchT.ini");
	csFiles.Add(L"WWizPatchEPS.ini");
	csFiles.Add(L"VibroPatchAS.ini");

	CString csRFolder;
	m_editRightFolder.GetWindowText(csRFolder);

	bool bMergeSuccess = true;

	for (DWORD dwCount = 0x00; dwCount < csFiles.GetCount(); dwCount++)
	{
		CString csFileName = csFiles[dwCount];

		CListCtrl *pObjList = NULL;
		switch (dwCount)
		{
		case 0:
			pObjList = &m_lstBasic;
			break;
		case 1:
			pObjList = &m_lstEssential;
			break;
		case 2:
			pObjList = &m_lstPro;
			break;
		case 3:
			pObjList = &m_lstElite;
			break;
		case 4:
			pObjList = &m_lstEliteServer;
			break;
		case 5:
			pObjList = &m_lstEssPlus;
			break;
		default:
			continue;
		}

		if (!MergeContents(csRFolder + L"\\" + csFileName, dwCount + 1, *pObjList))
		{
			bMergeSuccess = false;
			continue;
		}
	}

	m_btnCompare.EnableWindow(FALSE);
	m_btnMerge.EnableWindow(TRUE);

	if (bMergeSuccess)
		SetStatus(L"Merged successfully...");
	else
		SetStatus(L"Not merged properly, please check manually");
}

/***************************************************************************
Function Name  : IsItemsChecked
Description    : Function which takes list control object as input and check is any entries selected?
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::IsItemsChecked(CListCtrl &objlst)
{
	bool bReturn = false;
	for (DWORD dwIndex = 0x00; dwIndex < objlst.GetItemCount(); dwIndex++)
	{
		if (objlst.GetCheck(dwIndex))
		{
			bReturn = true;
			break;
		}
	}
	return bReturn;
}

/***************************************************************************
Function Name  : MergeContents
Description    : Function which takes file path, product ID, and merge one by one entry from list control
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::MergeContents(CString csFilePath, DWORD dwProdID, CListCtrl &objlst)
{
	bool bRetrun = true;

	DWORD dwItemCount = objlst.GetItemCount();

	if (dwItemCount == 0)
	{
		return true;
	}

	for (DWORD dwIndex = 0x00; dwIndex < objlst.GetItemCount(); dwIndex++)
	{
		if (objlst.GetCheck(dwIndex))
		{
			CString csSection;
			CString csKey;
			CString csData;

			csData = objlst.GetItemText(dwIndex, 0);
			csSection = objlst.GetItemText(dwIndex, 1);
			csKey = objlst.GetItemText(dwIndex, 2);

			if (csSection.Find(L"Version", 0) > 0)
			{
				CString csKey;
				if (csSection == L"ProductVersion")
				{
					csKey = L"ProductVer";
				}
				else if (csSection == L"DatabaseVersion")
				{
					csKey = L"DatabaseVer";
				}
				else if (csSection == L"DataEncVersion")
				{
					csKey = L"DataEncVer";
				}

				if (!WriteEntryInDestinationFile(csFilePath, csSection, csKey, csData))
				{
					bRetrun = false;
					return bRetrun;
				}
				continue;
			}

			//Copy here files to destination folder.
			if (!CopyFile2Destination(csData, csSection, dwProdID))
			{
				bRetrun = false;
				return bRetrun;
			}

			if (!WriteEntryInDestinationFile(csFilePath, csSection, csKey, csData))
			{
				bRetrun = false;
				return bRetrun;
			}
		}
	}
	return bRetrun;
}

/***************************************************************************
Function Name  : GetNewKeyFromDestinationFile
Description    : This function is for new entry, which provides new key.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::GetNewKeyFromDestinationFile(CString csFilePath, CString csSection, CString &csKey)
{
	bool bReturn = false;

	if (!PathFileExists(csFilePath))
	{
		return bReturn;
	}

	DWORD dwRetValue = GetPrivateProfileInt(csSection, L"Count", 0, csFilePath);
	csKey.Format(L"%d", dwRetValue);

	return dwRetValue != 0 ? bReturn = true : bReturn = false;
}

/***************************************************************************
Function Name  : WriteEntryInDestinationFile
Description    : Function to write value in destination file.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::WriteEntryInDestinationFile(CString csFilePath, CString csSection, CString csKey, CString csData)
{
	bool bReturn = false;

	if (!PathFileExists(csFilePath))
	{
		return bReturn;
	}

	DWORD dwRetValue = WritePrivateProfileString(csSection, csKey, csData, csFilePath);

	if (csSection.Find(L"Version", 0) > 0)
	{
		return dwRetValue != 0 ? bReturn = true : bReturn = false;
	}

	//Special case need to handle separately
	CString csCountSection = L"common";
	if (_tcscmp(csSection, L"Files_32") == 0)
		csCountSection = L"Count_32";
	else if (_tcscmp(csSection, L"Files_64") == 0)
		csCountSection = L"Count_64";
	else if (_tcscmp(csSection, L"CommonDB") == 0)
		csCountSection = L"CommonDB";

	DWORD dwValue = GetPrivateProfileInt(csCountSection, L"Count", 0, csFilePath);
	DWORD dwNewValue = _wtoi(csKey);
	if (dwNewValue > dwValue)
	{
		WritePrivateProfileString(csCountSection, L"Count", csKey, csFilePath);
	}

	return dwRetValue != 0 ? bReturn = true : bReturn = false;
}

/***************************************************************************
Function Name  : CopyFile2Destination
Description    : Function which copies file from source to destination
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::CopyFile2Destination(CString csData, CString csSection, DWORD dwProdID)
{
	bool bReturn = false;

	CString csLPath, csRPath;
	m_editLeftFolder.GetWindowText(csLPath);
	m_editRightFolder.GetWindowText(csRPath);

	TCHAR szExeName[MAX_PATH] = { 0 };
	TCHAR szZipSize[MAX_PATH] = { 0 };
	TCHAR szFullSize[MAX_PATH] = { 0 };
	TCHAR szFileHash[MAX_PATH] = { 0 };

	if (!ParseIniLine(csData.GetBuffer(), szExeName, szZipSize, szFullSize, szFileHash))
	{
		AddLogEntry(L"### ParseIniLine Failed: Data:%s", csData, 0, true);
		return bReturn;
	}

	//Special case need to handle separately
	CString csFolderName = L"Common";
	if (_tcscmp(csSection, L"Files_32") == 0)
		csFolderName = L"32";
	else if (_tcscmp(csSection, L"Files_64") == 0)
		csFolderName = L"64";
	else if (_tcscmp(csSection, L"CommonDB") == 0)
		csFolderName = L"CommonDB";

	CString csSourcePath, csDestinationPath;
	csSourcePath = csLPath + L"\\" + csFolderName + L"\\" + CString(szExeName) + L".zip";

	csDestinationPath = csRPath + L"\\" + csFolderName;
	if (!PathFileExists(csDestinationPath))
	{
		if (!CreateDirectory(csDestinationPath, NULL))
		{
			AddLogEntry(L"### CreateDirectory Failed: Path:%s", csDestinationPath.GetBuffer(), 0, true);
			return bReturn;
		}
	}

	csDestinationPath += L"\\" + CString(szExeName) + L".zip";

	if (!CopyFile(csSourcePath, csDestinationPath, FALSE))
	{
		AddLogEntry(L"### CopyFile Failed: SourcePath:%s, DestPath:%s", csSourcePath, csDestinationPath, 0, true);
		return bReturn;
	}

	CString csProdName;
	switch (dwProdID)
	{
	case 1:
		csProdName = L"Basic";
		break;
	case 2:
		csProdName = L"Essential";
		break;
	case 3:
		csProdName = L"Pro";
		break;
	case 4:
		csProdName = L"Elite";
		break;
	case 5:
		csProdName = L"Elite";
		break;
	case 6:
		csProdName = L"EssentialPlus";
		break;
	default:
		break;
	}

	csSourcePath = csLPath + L"\\" + csProdName + L"\\" + csFolderName + L"\\" + szExeName + L".zip";
	csDestinationPath = csRPath + L"\\" + csProdName + L"\\" + csFolderName;

	if (!PathFileExists(csDestinationPath))
	{
		if (!CreateDirectory(csDestinationPath, NULL))
		{
			AddLogEntry(L"### CreateDirectory Failed: Path:%s", csDestinationPath.GetBuffer(), 0, true);
			return bReturn;
		}
	}

	csDestinationPath += L"\\" + CString(szExeName) + L".zip";;

	if (!PathFileExists(csSourcePath))
	{
		AddLogEntry(L"### Path does not exists: %s", csSourcePath, 0, true);
		return bReturn;
	}

	if (!CopyFile(csSourcePath, csDestinationPath, FALSE))
	{
		AddLogEntry(L"### CopyFile Failed: SourcePath:%s, DestPath:%s", csSourcePath, csDestinationPath, 0, true);
		return bReturn;
	}

	bReturn = true;

	return bReturn;
}

/***************************************************************************
Function Name  : ISDifferenceWithSecondFile
Description    : Function which takes input value as section name and check is difference found.
Author Name    : Ram Shelke
Date           : 22 Mar 2016
SR_NO			 :
****************************************************************************/
bool CPatchMerger::ISDifferenceWithSecondFile(CString csFilePath, LPTSTR lpszIniLine, LPTSTR lpszKey, LPTSTR lpszData)
{
	bool bReturn = true;

	TCHAR szValueName[MAX_PATH] = { 0 };
	TCHAR	szValueData[2048] = { 0 };

	ZeroMemory(szValueName, sizeof(szValueName));
	GetPrivateProfileString(lpszIniLine, lpszKey, L"", szValueData, _countof(szValueData), csFilePath);

	if (_tcscmp(lpszData, szValueData) == 0)
	{
		bReturn = false;
	}

	return bReturn;
}