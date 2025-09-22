/********************************************************************************************************** 
   Program Name          : CommonFunctions.cpp
   Description           : Wrapper class to use common functions in product.
						   This class helps to maintain all commonly used functions.
   Author Name           : Niranjan Deshak
   Date Of Creation      : 1/31/2015
   Version No            : 1.8.3.0
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description	
***********************************************************************************************************/
#include "StdAfx.h"
#include "CommonFunctions.h"
#include "WardwizOSVersion.h"

#define FW(x,y) ::FindWindowEx(x, NULL, y, L"")

/***************************************************************************
  Function Name  : CCommonFunctions
  Description    : Const'r
  Author Name    : Niranjan Deshak
  S.R. No        : 
  Date           : 1/31/2015
****************************************************************************/
CCommonFunctions::CCommonFunctions(void)
{
}

/***************************************************************************
  Function Name  : CCommonFunctions
  Description    : Dist'r
  Author Name    : Niranjan Deshak
  S.R. No        : 
  Date           : 1/31/2015
****************************************************************************/
CCommonFunctions::~CCommonFunctions(void)
{
}
/***********************************************************************************************
  Function Name  : RefreshTaskbarNotificationArea
  Description    : The System tray will be refreshed to remove old Wardwiz Tray Icon.
  Author Name    : Niranjan Deshak
  SR.NO			 : 
  Date           : 29 Jan 2015
***********************************************************************************************/
void CCommonFunctions::RefreshTaskbarNotificationArea()
{
	try
	{
		HWND hNotificationArea  = NULL;
		RECT rect				= {0};

		CString csCaption;

		CWardWizOSversion objOSVersionWrap;
		DWORD OsType = objOSVersionWrap.DetectClientOSVersion();

		switch(OsType)
		{
		case WINOS_95:
			break;
		case WINOS_98:
			break;
		case WINOS_2000:
			break;
		case WINOS_NT:
			break;
		case WINOS_VISTA:
		case WINOS_XP:
		case WINOS_XP64: 
			//ISSUE NO - 8 After re- installation showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
			//NAME - Niranjan Deshak. - 2 feb 2015.
			csCaption = m_objwardwizLangManager.GetString(L"IDS_COMMON_ICON_CAPTION");// Windows XP and Windows vista
			//ISSUE NO - After installation of Patch Setup showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
			//NAME - Niranjan Deshak. - 10th Feb 2015.
			if(csCaption == L"")
			{
				csCaption = "Notification Area";
			}
			break;
		case WINOS_WIN7:
		case WINOS_WIN8:
		case WINOS_WIN8_1:
			//ISSUE NO - 8 After re- installation showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
			//NAME - Niranjan Deshak. - 2 feb 2015.
			csCaption = m_objwardwizLangManager.GetString(L"IDS_COMMON_ICON_CAPTION_WIN7");// Windows 7 and above
			//ISSUE NO - After installation of Patch Setup showing multiple icon of WardWizTray in task bar till  don’t move mouse cursor on system tray. 
			//NAME - Niranjan Deshak. - 10th Feb 2015.
			if(csCaption == L"")
			{
				csCaption = "User Promoted Notification Area";
			}
			break;
		case WINOSUNKNOWN_OR_NEWEST:
			MessageBox(NULL, L"NEWOS_OR_OLDOS", L"Vibranium", MB_OK|MB_ICONINFORMATION);
			break;
		}

		::GetClientRect(
			hNotificationArea = ::FindWindowEx(
			FW(FW(FW(NULL, L"Shell_TrayWnd"), L"TrayNotifyWnd"), L"SysPager"),
			NULL,
			L"ToolbarWindow32",
			// L"Notification Area"),
			csCaption), // Windows 7 and up
			&rect);

		for (LONG x = 0; x < rect.right; x += 5)
		{
			for (LONG y = 0; y < rect.bottom; y += 5)
			{	
				::SendMessage(hNotificationArea, WM_MOUSEMOVE, 0, (y << 16) + x);
			}
		}

		HWND hWnd = ::FindWindowA("NotifyIconOverflowWindow", NULL);
		if (hWnd)
			hWnd = ::FindWindowExA(hWnd, NULL, "ToolbarWindow32", "Overflow Notification Area");

		if (!hWnd)
			return;
		RECT rcClient;
		BOOL bRet = ::GetClientRect(hWnd, &rcClient);
		if (!bRet)
			return;
		for (int y = rcClient.bottom - 16; y >= 0; y -= 16)
		for (int x = rcClient.right - 16; x >= 0; x -= 16)
			::PostMessageA(hWnd, WM_MOUSEMOVE, NULL, MAKELPARAM(x, y));


	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CCommonFunctions::RefreshTaskbarNotificationArea", 0, 0, true, SECONDLEVEL);
	}
}