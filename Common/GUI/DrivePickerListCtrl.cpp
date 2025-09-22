/***************************************************************************                      
   Program Name          : DrivePickerListCtrl.cpp
   Description           : List control for list out drives existing in computer
   Author Name           :                                                                         
   Date Of Creation      : 18th Oct,2013
   Version No            : 1.0.0.1
   Special Logic Used    : 
   Modification Log      :           
****************************************************************************/
#include "stdafx.h"
#include "DrivePickerListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***************************************************************************************************                    
*  Function Name  : CDrivePickerListCtrl                                                     
*  Description    : C'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0307
*  Date           : 18 Oct 2013
****************************************************************************************************/
CDrivePickerListCtrl::CDrivePickerListCtrl()
{
}

/***************************************************************************************************                    
*  Function Name  : ~CDrivePickerListCtrl                                                     
*  Description    : D'tor
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0308
*  Date           : 18 Oct 2013
****************************************************************************************************/
CDrivePickerListCtrl::~CDrivePickerListCtrl()
{
	if(NULL != m_ImgList.GetSafeHandle())
	{
		m_ImgList.Detach();
	}
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0309
*  Date           : 18 Oct 2013
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CDrivePickerListCtrl, CListCtrl)
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : InitList                                                     
*  Description    : Initialized list
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0310
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::InitList(int nIconSize, DWORD dwFlags)
{
	// Only 16x16 and 32x32 sizes are supported now.
	ASSERT(16 == nIconSize  ||  32 == nIconSize);
	// Check that only valid flags were passed in.
	ASSERT((dwFlags & DDS_DLIL_ALL_DRIVES) == dwFlags);
	CommonInit(32 == nIconSize, dwFlags);
}

/***************************************************************************************************                    
*  Function Name  : SetSelection                                                     
*  Description    : Set selection mark on drives check box
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0311
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::SetSelection(const DWORD dwDrives)
{
	int     nIndex;
	int     nMaxIndex = GetItemCount() - 1;
	TCHAR   cNextDrive;
	BOOL    bCheck;

	// Verify that the only bits set are bits 0 thru 25.
	ASSERT((dwDrives &  0x3FFFFFF) == dwDrives);

	for(nIndex = 0; nIndex <= nMaxIndex; nIndex++)
	{
		cNextDrive = static_cast<TCHAR>(GetItemData(nIndex));
		// Internal check - letters are always kept as uppercase.
		ASSERT(_istupper(cNextDrive));
		bCheck =(dwDrives & (1 << (cNextDrive - 'A')));
		SetCheck(nIndex, bCheck);
	}
}

/***************************************************************************************************                    
*  Function Name  : UnCheckAll                                                     
*  Description    : Unchecks all drives check box
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0312
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::UnCheckAll()
{
	int     nIndex;
	int     nMaxIndex = GetItemCount() - 1;
	for(nIndex = 0; nIndex <= nMaxIndex; nIndex++)
	{
		SetCheck(nIndex, false);
	}
}

/***************************************************************************************************                    
*  Function Name  : SetSelection                                                     
*  Description    : Set selection mark on drives check box.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0313
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::SetSelection(LPCTSTR szDrives)
{
	int     nIndex;
	int     nMaxIndex = GetItemCount() - 1;
	CString sDrives;
	TCHAR   cNextDrive;
	BOOL    bCheck;

#ifdef _DEBUG
	LPCTSTR pchNextDrive = szDrives;
	ASSERT(AfxIsValidString(szDrives));

	// Verify that only letters a thru z are in szDrives.

	for(; '\0' != *pchNextDrive; pchNextDrive++)
	{
		ASSERT(_totupper(*pchNextDrive) >= 'A' &&
			_totupper(*pchNextDrive)<= 'Z');
	}
#endif

	sDrives = szDrives;
	sDrives.MakeUpper();

	for(nIndex = 0; nIndex <= nMaxIndex; nIndex++)
	{
		cNextDrive = static_cast<TCHAR>(GetItemData(nIndex));
		// Internal check - letters are always kept as uppercase.
		ASSERT(_istupper(cNextDrive));
		bCheck =(-1 != sDrives.Find(cNextDrive));
		SetCheck(nIndex, bCheck);
	}
}


/***************************************************************************************************                    
*  Function Name  : GetNumSelectedDrives                                                     
*  Description    : Set number of selected drives.
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0314
*  Date           : 18 Oct 2013
****************************************************************************************************/
BYTE CDrivePickerListCtrl::GetNumSelectedDrives()const
{
	BYTE byNumDrives = 0;
	int  nIndex;
	int  nMaxIndex = GetItemCount() - 1;

	for(nIndex = 0; nIndex <= nMaxIndex; nIndex++)
	{
		if(GetCheck(nIndex))
		{
			byNumDrives++;
		}
	}
	return byNumDrives;
}

/***************************************************************************************************                    
*  Function Name  : GetSelectedDrives                                                     
*  Description    : Gets all selected drives
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0315
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::GetSelectedDrives(DWORD* pdwSelectedDrives)const
{
	int nIndex;
	int nMaxIndex = GetItemCount() - 1;
	TCHAR chDrive;

	ASSERT(AfxIsValidAddress(pdwSelectedDrives, sizeof(DWORD)));

	*pdwSelectedDrives = 0;

	for(nIndex = 0; nIndex <= nMaxIndex; nIndex++)
	{
		if(GetCheck(nIndex))
		{
			// Retrieve the drive letter and convert it to the right bit
			// in the DWORD bitmask.

			chDrive = static_cast<TCHAR>(GetItemData(nIndex));

			// Internal check - letters are always kept as uppercase.
			ASSERT(_istupper(chDrive));

			*pdwSelectedDrives |=(1 <<(chDrive - 'A'));
		}
	}
}

/***************************************************************************************************                    
*  Function Name  : GetSelectedDrives                                                     
*  Description    : Gets all selected drives
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0316
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::GetSelectedDrives(LPTSTR szSelectedDrives)const
{
	int   nIndex;
	int   nMaxIndex = GetItemCount() - 1;
	TCHAR szDrive[2] ={0, 0 };

	// Need at least a 27-char buffer to hold A-Z and the terminating zero.
	ASSERT(AfxIsValidAddress(szSelectedDrives, 27 * sizeof(TCHAR)));

	*szSelectedDrives = 0;

	for(nIndex = 0; nIndex <= nMaxIndex; nIndex++)
	{
		if(GetCheck(nIndex))
		{
			*szDrive = static_cast<TCHAR>(GetItemData(nIndex));

			lstrcat(szSelectedDrives, szDrive);
		}
	}
}

/***************************************************************************************************                    
*  Function Name  : CommonInit                                                     
*  Description    : Initalized common list control
*  Author Name    : 
*  SR_NO          : WRDWIZCOMMON_0317
*  Date           : 18 Oct 2013
****************************************************************************************************/
void CDrivePickerListCtrl::CommonInit(BOOL bLargeIcons, DWORD dwFlags)
{
	DWORD dwRemoveStyles = LVS_TYPEMASK | LVS_SORTASCENDING | LVS_SORTDESCENDING |
		LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS;
	DWORD dwNewStyles = LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS |
		LVS_SINGLESEL;

	DWORD dwRemoveExStyles = 0;
	DWORD dwNewExStyles = LVS_EX_CHECKBOXES;

	ASSERT(0  == (GetStyle()& LVS_OWNERDATA));

	ModifyStyle(dwRemoveStyles, dwNewStyles);
	ListView_SetExtendedListViewStyleEx(GetSafeHwnd(),
		dwRemoveExStyles | dwNewExStyles,
		dwNewExStyles);
	DeleteAllItems();

	LVCOLUMN rCol ={LVCF_WIDTH };

	if(!GetColumn(0, &rCol))
	{
		InsertColumn(0, _T(""));
	}


	// Get the handle of the system image list - which list we retrieve
	// (large or small icons)is controlled by the bLargeIcons parameter.

	SHFILEINFO sfi;
	DWORD_PTR	dwRet;
	HIMAGELIST hImgList;
	UINT       uFlags = SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX;

	uFlags |=(bLargeIcons ? SHGFI_LARGEICON : SHGFI_SMALLICON);
	dwRet = SHGetFileInfo(_T("buffy.com"), FILE_ATTRIBUTE_NORMAL,
		&sfi, sizeof(SHFILEINFO), uFlags);

	DestroyIcon(sfi.hIcon);
	hImgList = reinterpret_cast<HIMAGELIST>(dwRet);
	ASSERT(NULL != hImgList);

	if(NULL != m_ImgList.GetSafeHandle())
		m_ImgList.Detach();

	VERIFY(m_ImgList.Attach(hImgList));

	SetImageList(&m_ImgList, LVSIL_SMALL);
	TCHAR szDriveRoot[] = _T("x:\\");
	TCHAR cDrive;
	DWORD dwDrivesOnSystem = GetLogicalDrives();
	int   nIndex = 0;
	UINT  uDriveType;

	// Sanity check - if this assert fires, how the heck did you boot? :)
	ASSERT(0 != dwDrivesOnSystem);


	uFlags = SHGFI_SYSICONINDEX | SHGFI_DISPLAYNAME | SHGFI_ICON;
	uFlags |=(bLargeIcons ? SHGFI_LARGEICON : SHGFI_SMALLICON);

	for(cDrive = 'A'; cDrive <= 'Z'; cDrive++, dwDrivesOnSystem >>= 1)
	{
		if(!(dwDrivesOnSystem & 1))
			continue;

		// Get the type for the next drive, and check dwFlags to determine
		// if we should show it in the list.

		szDriveRoot[0] = cDrive;

		uDriveType = GetDriveType(szDriveRoot);

		switch(uDriveType)
		{
		case DRIVE_NO_ROOT_DIR:
		case DRIVE_UNKNOWN:
			// Skip disconnected network drives and drives that Windows
			// can't figure out.
			continue;
			break;

		case DRIVE_REMOVABLE:
			if(!(dwFlags & DDS_DLIL_REMOVABLES))
				continue;
			break;

		case DRIVE_FIXED:
			if(!(dwFlags & DDS_DLIL_HARDDRIVES))
				continue;
			break;

		case DRIVE_REMOTE:
			if(!(dwFlags & DDS_DLIL_NETDRIVES))
				continue;
			break;

		case DRIVE_CDROM:
			if(!(dwFlags & DDS_DLIL_CDROMS))
				continue;
			break;

		case DRIVE_RAMDISK:
			if(!(dwFlags & DDS_DLIL_RAMDRIVES))
				continue;
			break;

			DEFAULT_UNREACHABLE;
		}
		if(SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO),
			uFlags))
		{
			InsertItem(nIndex, sfi.szDisplayName, sfi.iIcon);
			SetItemData(nIndex, cDrive);
			nIndex++;
			DestroyIcon(sfi.hIcon);
		}
	}   // end for

	SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
}
