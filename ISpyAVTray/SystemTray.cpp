/////////////////////////////////////////////////////////////////////////////
// SystemTray.cpp : implementation file
//
// MFC VERSION
//
// This is a conglomeration of ideas from the MSJ "Webster" application,
// sniffing round the online docs, and from other implementations such
// as PJ Naughter's "CTrayNotifyIcon" (http://indigo.ie/~pjn/ntray.html)
// especially the "CSystemTray::OnTrayNotification" member function.
// Joerg Koenig suggested the icon animation stuff
//
// This class is a light wrapper around the windows system tray stuff. It
// adds an icon to the system tray with the specified ToolTip text and 
// callback notification value, which is sent back to the Parent window.
//
// The tray icon can be instantiated using either the constructor or by
// declaring the object and creating (and displaying) it later on in the
// program. eg.
//
//        CSystemTray m_SystemTray;    // Member variable of some class
//        
//        ... 
//        // in some member function maybe...
//        m_SystemTray.Create(pParentWnd, WM_MY_NOTIFY, "Click here", 
//                          hIcon, nSystemTrayID);
//
// Written by Chris Maunder (cmaunder@mail.com)
// Copyright (c) 1998-2003.
//
// Updated: 25 Jul 1998 - Added icon animation, and derived class
//                        from CWnd in order to handle messages. (CJM)
//                        (icon animation suggested by Joerg Koenig.
//                        Added API to set default menu item. Code provided
//                        by Enrico Lelina.
//
// Updated: 6 June 1999 - SetIcon can now load non-standard sized icons (Chip Calvert)
//                        Added "bHidden" parameter when creating icon
//                        (Thanks to Michael Gombar for these suggestions)
//                        Restricted tooltip text to 64 characters.
//
// Updated: 9 Nov 1999  - Now works in WindowsCE.
//                        Fix for use in NT services (Thomas Mooney, TeleProc, Inc)
//                        Added W2K stuff by Michael Dunn
//
// Updated: 1 Jan 2000  - Added tray minimisation stuff.
// 
// Updated: 21 Sep 2000 - Added GetDoWndAnimation - animation only occurs if the system
//                        settings allow it (Matthew Ellis). Updated the GetTrayWndRect
//                        function to include more fallback logic (Matthew Ellis)
//                        NOTE: Signature of GetTrayWndRect has changed!
// 
// Updated: 4 Aug 2003 - Fixed bug that was stopping icon from being recreated when
//                       Explorer crashed
//                       Fixed resource leak in SetIcon
//						 Animate() now checks for empty icon list - Anton Treskunov
//						 Added the virutal CustomizeMenu() method - Anton Treskunov
//                       
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage caused through use.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 
//
/////////////////////////////////////////////////////////////////////////////
   
// SystemTray.cpp : Defines the class behaviors for the application.
/***************************************************************
*  Program Name: SystemTray.cpp                                                                                                    
*  Description: For right pop up
*  Author Name: Nitin                                                                                                    
*  Date Of Creation: 18th Mar ,2014
*  Version No: 1.0.0.2
*****************************************************************/

#include "stdafx.h"
#include "SystemTray.h"
#include "ISpyAVTray.h"
#include "ISpyAVTrayDlg.h"
#include "ISpyCommunicator.h"
#include <Msi.h>

void closeToolTipWindow(void);
LPCTSTR g_GUIDofInstalledOutllookH[10]={L"11",L"12",L"14",L"15"};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _WIN32_WCE  // Use C++ exception handling instead of structured.
#undef TRY
#undef CATCH
#undef END_CATCH
#define TRY try
#define CATCH(ex_class, ex_object) catch(ex_class* ex_object)
#define END_CATCH
#endif  // _WIN32_WCE

#ifndef _countof
#define _countof(x) ( sizeof(x) / sizeof(x[0]) )
#endif

IMPLEMENT_DYNAMIC(CSystemTray, CWnd)
bool SendEnableEmailScanMessage2UI(DWORD dwSelectedOption);
const UINT CSystemTray::m_nTimerID    = 4567;
UINT CSystemTray::m_nMaxTooltipLength  = 2048;     // This may change...
const UINT CSystemTray::m_nTaskbarCreatedMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));
CWnd  CSystemTray::m_wndInvisible;

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

// CISpyAVTrayApp
/***************************************************************************************************                    
*  Function Name  : CSystemTray                                                     
*  Description    : C'tor
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0079
*  Date           : 18th Mar ,2014
****************************************************************************************************/
CSystemTray::CSystemTray()
{
    Initialise();
}

/***************************************************************************************************                    
*  Function Name  : CSystemTray                                                     
*  Description    : parameterized C'tor
*  Author Name    : Nitin
*  SR_NO		  :  WRDWIZTRAY_0080
*  Date           : 18th Mar ,2014
****************************************************************************************************/

CSystemTray::CSystemTray(CWnd* pParent,             // The window that will recieve tray notifications
                         UINT uCallbackMessage,     // the callback message to send to parent
                         LPCTSTR szToolTip,         // tray icon tooltip
                         HICON icon,                // Handle to icon
                         UINT uID,                  // Identifier of tray icon
                         BOOL bHidden /*=FALSE*/,   // Hidden on creation?                  
                         LPCTSTR szBalloonTip /*=NULL*/,    // Ballon tip (w2k only)
                         LPCTSTR szBalloonTitle /*=NULL*/,  // Balloon tip title (w2k)
                         DWORD dwBalloonIcon /*=NIIF_NONE*/,// Ballon tip icon (w2k)
                         UINT uBalloonTimeout /*=10*/)      // Balloon timeout (w2k)
{
    Initialise();
    Create(pParent, uCallbackMessage, szToolTip, icon, uID, bHidden,
           szBalloonTip, szBalloonTitle, dwBalloonIcon, uBalloonTimeout);
}

/***************************************************************************************************                    
*  Function Name  : Initialise                                                     
*  Description    : Initializing variable
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0081
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::Initialise()
{
    memset(&m_tnd, 0, sizeof(m_tnd));

    m_bEnabled = FALSE;
    m_bHidden  = TRUE;
    m_bRemoved = TRUE;
	m_bExitPopupflag = false;

    m_DefaultMenuItemID    = 0;
    m_DefaultMenuItemByPos = TRUE;

    m_bShowIconPending = FALSE;

    m_uIDTimer   = 0;
    m_hSavedIcon = NULL;

	m_pTargetWnd = NULL;
	m_uCreationFlags = 0;
	m_sMenu = NULL;

	m_Key = CWWizSettingsWrapper::GetProductRegistryKey();
	SetRegistryFlag();
	m_bOutlookFlag = false;


#ifdef SYSTEMTRAY_USEW2K
    OSVERSIONINFO os = { sizeof(os) };
    GetVersionEx(&os);
    m_bWin2K = ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5 );
#else
    m_bWin2K = FALSE;
#endif
}

// update by Michael Dunn, November 1999
//
//  New version of Create() that handles new features in Win 2K.
//
// Changes:
//  szTip: Same as old, but can be 128 characters instead of 64.
//  szBalloonTip: Text for a balloon tooltip that is shown when the icon
//                is first added to the tray.  Pass "" if you don't want
//                a balloon.
//  szBalloonTitle: Title text for the balloon tooltip.  This text is shown
//                  in bold above the szBalloonTip text.  Pass "" if you
//                  don't want a title.
//  dwBalloonIcon: Specifies which icon will appear in the balloon.  Legal
//                 values are:
//                     NIIF_NONE: No icon
//                     NIIF_INFO: Information
//                     NIIF_WARNING: Exclamation
//                     NIIF_ERROR: Critical error (red circle with X)
//  uBalloonTimeout: Number of seconds for the balloon to remain visible.
//                   Must be between 10 and 30 inclusive.

/***************************************************************************************************                    
*  Function Name  : Create                                                     
*  Description    : Create system tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0082
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip, 
                         HICON icon, UINT uID, BOOL bHidden /*=FALSE*/,
                         LPCTSTR szBalloonTip /*=NULL*/, 
                         LPCTSTR szBalloonTitle /*=NULL*/,  
                         DWORD dwBalloonIcon /*=NIIF_NONE*/,
                         UINT uBalloonTimeout /*=10*/)
{
#ifdef _WIN32_WCE
    m_bEnabled = TRUE;
#else
    // this is only for Windows 95 (or higher)
    m_bEnabled = (GetVersion() & 0xff) >= 4;
    if (!m_bEnabled) 
    {
        ASSERT(FALSE);
        return FALSE;
    }
#endif

    //m_nMaxTooltipLength = _countof(m_tnd.szTip);
    
    // Make sure we avoid conflict with other messages
    ASSERT(uCallbackMessage >= WM_APP);

    // Tray only supports tooltip text up to m_nMaxTooltipLength) characters
    ASSERT(AfxIsValidString(szToolTip));
    ASSERT(_tcslen(szToolTip) <= m_nMaxTooltipLength);

    // Create an invisible window

	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP, 0, 0, 0, 0, NULL, 0))
	{
		DWORD dwLastError = GetLastError();
		CString csErrMsg; 
		csErrMsg.Format(L"%d", dwLastError);
		AddLogEntry(L"Invisible window is not created with in CSystemTray::Create, err %s", csErrMsg, 0, true, SECONDLEVEL);
	}
	 
	

    // load up the NOTIFYICONDATA structure
    m_tnd.cbSize = sizeof(NOTIFYICONDATA);
    m_tnd.hWnd   = pParent->GetSafeHwnd()? pParent->GetSafeHwnd() : m_hWnd;
    m_tnd.uID    = uID;
    m_tnd.hIcon  = icon;
    m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_tnd.uCallbackMessage = uCallbackMessage;
    _tcsncpy_s(m_tnd.szTip, szToolTip, m_nMaxTooltipLength-1);

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K && szBalloonTip)
    {
        // The balloon tooltip text can be up to 255 chars long.
        ASSERT(AfxIsValidString(szBalloonTip));
        ASSERT(lstrlen(szBalloonTip) < 256);

        // The balloon title text can be up to 63 chars long.
        if (szBalloonTitle)
        {
            ASSERT(AfxIsValidString(szBalloonTitle));
            ASSERT(lstrlen(szBalloonTitle) < 64);
        }

        // dwBalloonIcon must be valid.
        ASSERT(NIIF_NONE == dwBalloonIcon    || NIIF_INFO == dwBalloonIcon ||
               NIIF_WARNING == dwBalloonIcon || NIIF_ERROR == dwBalloonIcon);

        // The timeout must be between 10 and 30 seconds.
        ASSERT(uBalloonTimeout >= 10 && uBalloonTimeout <= 30);

        m_tnd.uFlags |= NIF_INFO;

        _tcsncpy_s(m_tnd.szInfo, szBalloonTip, 255);
        if (szBalloonTitle)
            _tcsncpy_s(m_tnd.szInfoTitle, szBalloonTitle, 63);
        else
            m_tnd.szInfoTitle[0] = _T('\0');
        m_tnd.uTimeout    = uBalloonTimeout * 1000; // convert time to ms
        m_tnd.dwInfoFlags = dwBalloonIcon;
    }
#endif

    m_bHidden = bHidden;

#ifdef SYSTEMTRAY_USEW2K    
    if (m_bWin2K && m_bHidden)
    {
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = NIS_HIDDEN;
        m_tnd.dwStateMask = NIS_HIDDEN;
    }
#endif

	m_uCreationFlags = m_tnd.uFlags;	// Store in case we need to recreate in OnTaskBarCreate

    BOOL bResult = TRUE;
    if (!m_bHidden || m_bWin2K)
    {
        bResult = Shell_NotifyIcon(NIM_ADD, &m_tnd);
        m_bShowIconPending = m_bHidden = m_bRemoved = !bResult;
    }
    
#ifdef SYSTEMTRAY_USEW2K    
    if (m_bWin2K && szBalloonTip)
    {
        // Zero out the balloon text string so that later operations won't redisplay
        // the balloon.
        m_tnd.szInfo[0] = _T('\0');
    }
#endif

    return bResult;
}

/***************************************************************************************************                    
*  Function Name  : ~CSystemTray                                                     
*  Description    : D,tor
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0083
*  Date           : 18th Mar ,2014
****************************************************************************************************/
CSystemTray::~CSystemTray()
{
    RemoveIcon();
    m_IconList.RemoveAll();
    DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray icon manipulation

//////////////////////////////////////////////////////////////////////////
//
// Function:    SetFocus()
//
// Description:
//  Sets the focus to the tray icon.  Microsoft's Win 2K UI guidelines
//  say you should do this after the user dismisses the icon's context
//  menu.
//
// Input:
//  Nothing.
//
// Returns:
//  Nothing.
//
//////////////////////////////////////////////////////////////////////////
// Added by Michael Dunn, November, 1999
//////////////////////////////////////////////////////////////////////////

/***************************************************************************************************                    
*  Function Name  : SetFocus                                                     
*  Description    : The framework calls this member function after gaining the input focus
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0084
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::SetFocus()
{
#ifdef SYSTEMTRAY_USEW2K
    Shell_NotifyIcon ( NIM_SETFOCUS, &m_tnd );
#endif
}

/***************************************************************************************************                    
*  Function Name  : MoveToRight                                                     
*  Description    : Move to Right
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0085
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::MoveToRight()
{
    RemoveIcon();
    return AddIcon();
}

/***************************************************************************************************                    
*  Function Name  : AddIcon                                                     
*  Description    : Add Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0086
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::AddIcon()
{
    if (!m_bRemoved)
        RemoveIcon();

    if (m_bEnabled)
    {
        m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        if (!Shell_NotifyIcon(NIM_ADD, &m_tnd))
            m_bShowIconPending = TRUE;
        else
            m_bRemoved = m_bHidden = FALSE;
    }
    return (m_bRemoved == FALSE);
}

/***************************************************************************************************                    
*  Function Name  : RemoveIcon                                                     
*  Description    : Remove Icons from the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0087
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::RemoveIcon()
{
    m_bShowIconPending = FALSE;

    if (!m_bEnabled || m_bRemoved)
        return TRUE;

    m_tnd.uFlags = 0;
    if (Shell_NotifyIcon(NIM_DELETE, &m_tnd))
        m_bRemoved = m_bHidden = TRUE;

    return (m_bRemoved == TRUE);
}

/***************************************************************************************************                    
*  Function Name  : HideIcon                                                     
*  Description    : Hide Icons from the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0088
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::HideIcon()
{
    if (!m_bEnabled || m_bRemoved || m_bHidden)
        return TRUE;

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K)
    {
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = NIS_HIDDEN;
        m_tnd.dwStateMask = NIS_HIDDEN;

        m_bHidden = Shell_NotifyIcon( NIM_MODIFY, &m_tnd);
    }
    else
#endif
        RemoveIcon();

    return (m_bHidden == TRUE);
}

/***************************************************************************************************                    
*  Function Name  : ShowIcon                                                     
*  Description    : Show Icons from the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0089
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::ShowIcon()
{
    if (m_bRemoved)
        return AddIcon();

    if (!m_bHidden)
        return TRUE;

#ifdef SYSTEMTRAY_USEW2K
    if (m_bWin2K)
    {
        m_tnd.uFlags = NIF_STATE;
        m_tnd.dwState = 0;
        m_tnd.dwStateMask = NIS_HIDDEN;
        Shell_NotifyIcon ( NIM_MODIFY, &m_tnd );
        m_bHidden = FALSE;
    }
    else
#endif
        AddIcon();

    return (m_bHidden == FALSE);
}

/***************************************************************************************************                    
*  Function Name  : SetIcon                                                     
*  Description    : Set Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0090
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetIcon(HICON hIcon)
{
    if (!m_bEnabled)
        return FALSE;

    m_tnd.uFlags = NIF_ICON;
    m_tnd.hIcon = hIcon;

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

/***************************************************************************************************                    
*  Function Name  : SetIcon                                                     
*  Description    : Set Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0091
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetIcon(LPCTSTR lpszIconName)
{
    HICON hIcon = (HICON) ::LoadImage(AfxGetResourceHandle(), 
                                      lpszIconName,
                                      IMAGE_ICON, 
                                      0, 0,
                                      LR_DEFAULTCOLOR);
	if (!hIcon)
		return FALSE;

	BOOL bSuccess = SetIcon(hIcon);
	::DestroyIcon(hIcon);

    return bSuccess;
}

/***************************************************************************************************                    
*  Function Name  : SetIcon                                                     
*  Description    : Set Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0092
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetIcon(UINT nIDResource)
{
    return SetIcon(MAKEINTRESOURCE(nIDResource));
}

/***************************************************************************************************                    
*  Function Name  : SetStandardIcon                                                     
*  Description    : Set the standard Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0093
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetStandardIcon(LPCTSTR lpIconName)
{
    HICON hIcon = LoadIcon(NULL, lpIconName);

    return SetIcon(hIcon);
}

/***************************************************************************************************                    
*  Function Name  : SetStandardIcon                                                     
*  Description    : Set the standard Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0094
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetStandardIcon(UINT nIDResource)
{
	return SetStandardIcon(MAKEINTRESOURCE(nIDResource));
}

/***************************************************************************************************                    
*  Function Name  : GetIcon                                                     
*  Description    : Get Icons to the tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0095
*  Date           : 18th Mar ,2014
****************************************************************************************************/
HICON CSystemTray::GetIcon() const
{
    return (m_bEnabled)? m_tnd.hIcon : NULL;
}

/***************************************************************************************************                    
*  Function Name  : SetIconList                                                     
*  Description    : Set Icons in the List
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0096
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID) 
{
	if (uFirstIconID > uLastIconID)
        return FALSE;

	const CWinApp* pApp = AfxGetApp();
    if (!pApp)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_IconList.RemoveAll();
    TRY {
	    for (UINT i = uFirstIconID; i <= uLastIconID; i++)
		    m_IconList.Add(pApp->LoadIcon(i));
    }
    CATCH(CMemoryException, e)
    {
        e->ReportError();
        e->Delete();
        m_IconList.RemoveAll();
        return FALSE;
    }
    END_CATCH

    return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : SetIconList                                                     
*  Description    : Set Icons in the List
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0097
*  Date           : 18th Mar ,2014
****************************************************************************************************/

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons)
{
    m_IconList.RemoveAll();

    TRY {
	    for (UINT i = 0; i < nNumIcons; i++)
		    m_IconList.Add(pHIconList[i]);
    }
    CATCH (CMemoryException, e)
    {
        e->ReportError();
        e->Delete();
        m_IconList.RemoveAll();
        return FALSE;
    }
    END_CATCH

    return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : Animate                                                     
*  Description    : Animating Icons.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0098
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::Animate(UINT nDelayMilliSeconds, int nNumSeconds /*=-1*/)
{
	if (m_IconList.IsEmpty())
		return FALSE;

    StopAnimation();

    m_nCurrentIcon = 0;
    m_StartTime = COleDateTime::GetCurrentTime();
    m_nAnimationPeriod = nNumSeconds;
    m_hSavedIcon = GetIcon();

	// Setup a timer for the animation
	m_uIDTimer = SetTimer(m_nTimerID, nDelayMilliSeconds, NULL);

    return (m_uIDTimer != 0);
}

/***************************************************************************************************                    
*  Function Name  : StepAnimation                                                     
*  Description    : Show Step For Animation.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0099
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::StepAnimation()
{
    if (!m_IconList.GetSize())
        return FALSE;

    m_nCurrentIcon++;
    if (m_nCurrentIcon >= m_IconList.GetSize())
        m_nCurrentIcon = 0;

    return SetIcon(m_IconList[m_nCurrentIcon]);
}

/***************************************************************************************************                    
*  Function Name  : StopAnimation                                                     
*  Description    : Stop the Animation.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0100
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::StopAnimation()
{
    BOOL bResult = FALSE;

    if (m_uIDTimer)
	    bResult = KillTimer(m_uIDTimer);
    m_uIDTimer = 0;

    if (m_hSavedIcon)
        SetIcon(m_hSavedIcon);
    m_hSavedIcon = NULL;

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray tooltip text manipulation

/***************************************************************************************************                    
*  Function Name  : SetTooltipText                                                     
*  Description    : Set the tool tip text.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0101
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetTooltipText(LPCTSTR pszTip)
{
    ASSERT(AfxIsValidString(pszTip)); // (md)
    ASSERT(_tcslen(pszTip) < m_nMaxTooltipLength);

    if (!m_bEnabled) 
        return FALSE;

    m_tnd.uFlags = NIF_TIP;
    _tcsncpy_s(m_tnd.szTip, pszTip, m_nMaxTooltipLength-1);

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

/***************************************************************************************************                    
*  Function Name  : SetTooltipText                                                     
*  Description    : Set the tool tip text.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0102
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetTooltipText(UINT nID)
{
    CString strText;
    VERIFY(strText.LoadString(nID));

    return SetTooltipText(strText);
}

/***************************************************************************************************                    
*  Function Name  : GetTooltipText                                                     
*  Description    : Get the tool tip text.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0103
*  Date           : 18th Mar ,2014
****************************************************************************************************/
CString CSystemTray::GetTooltipText() const
{
    CString strText;
    if (m_bEnabled)
        strText = m_tnd.szTip;

    return strText;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray support for Win 2K features.

//////////////////////////////////////////////////////////////////////////
//
// Function:    ShowBalloon
//
// Description:
//  Shows a balloon tooltip over the tray icon.
//
// Input:
//  szText: [in] Text for the balloon tooltip.
//  szTitle: [in] Title for the balloon.  This text is shown in bold above
//           the tooltip text (szText).  Pass "" if you don't want a title.
//  dwIcon: [in] Specifies an icon to appear in the balloon.  Legal values are:
//                 NIIF_NONE: No icon
//                 NIIF_INFO: Information
//                 NIIF_WARNING: Exclamation
//                 NIIF_ERROR: Critical error (red circle with X)
//  uTimeout: [in] Number of seconds for the balloon to remain visible.  Can
//            be between 10 and 30 inclusive.
//
// Returns:
//  TRUE if successful, FALSE if not.
//
//////////////////////////////////////////////////////////////////////////
// Added by Michael Dunn, November 1999
//////////////////////////////////////////////////////////////////////////

/***************************************************************************************************                    
*  Function Name  : ShowBalloon                                                     
*  Description    : Show Ballon.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0104
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::ShowBalloon(LPCTSTR szText,
                              LPCTSTR szTitle  /*=NULL*/,
                              DWORD   dwIcon   /*=NIIF_NONE*/,
                              UINT    uTimeout /*=10*/ )
{
#ifndef SYSTEMTRAY_USEW2K
    return FALSE;
#else
    // Bail out if we're not on Win 2K.
    if (!m_bWin2K)
        return FALSE;

    // Verify input parameters.

    // The balloon tooltip text can be up to 255 chars long.
    ASSERT(AfxIsValidString(szText));
    ASSERT(lstrlen(szText) < 256);

    // The balloon title text can be up to 63 chars long.
    if (szTitle)
    {
        ASSERT(AfxIsValidString( szTitle));
        ASSERT(lstrlen(szTitle) < 64);
    }

    // dwBalloonIcon must be valid.
    ASSERT(NIIF_NONE == dwIcon    || NIIF_INFO == dwIcon ||
           NIIF_WARNING == dwIcon || NIIF_ERROR == dwIcon);

    // The timeout must be between 10 and 30 seconds.
    ASSERT(uTimeout >= 10 && uTimeout <= 30);


    m_tnd.uFlags = NIF_INFO;
    _tcsncpy_s(m_tnd.szInfo, szText, 256);
    if (szTitle)
        _tcsncpy_s(m_tnd.szInfoTitle, szTitle, 64);
    else
        m_tnd.szInfoTitle[0] = _T('\0');
    m_tnd.dwInfoFlags = dwIcon;
    m_tnd.uTimeout = uTimeout * 1000;   // convert time to ms

    BOOL bSuccess = Shell_NotifyIcon (NIM_MODIFY, &m_tnd);

    // Zero out the balloon text string so that later operations won't redisplay
    // the balloon.
    m_tnd.szInfo[0] = _T('\0');

    return bSuccess;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification window stuff

/***************************************************************************************************                    
*  Function Name  : SetNotificationWnd                                                     
*  Description    : Set Notification Window.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0105
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetNotificationWnd(CWnd* pWnd)
{
    if (!m_bEnabled) 
        return FALSE;

    // Make sure Notification window is valid
    if (!pWnd || !::IsWindow(pWnd->GetSafeHwnd()))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_tnd.hWnd = pWnd->GetSafeHwnd();
    m_tnd.uFlags = 0;

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

/***************************************************************************************************                    
*  Function Name  : GetNotificationWnd                                                     
*  Description    : Get Notification Window.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0106
*  Date           : 18th Mar ,2014
****************************************************************************************************/
CWnd* CSystemTray::GetNotificationWnd() const
{
    return CWnd::FromHandle(m_tnd.hWnd);
}

// Hatr added

// Hatr added

// Change or retrive the window to send menu commands to
/***************************************************************************************************                    
*  Function Name  : SetTargetWnd                                                     
*  Description    : Set Target Window
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0107
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetTargetWnd(CWnd* pTargetWnd)
{
    m_pTargetWnd = pTargetWnd;
    return TRUE;
} // CSystemTray::SetTargetWnd()

/***************************************************************************************************                    
*  Function Name  : GetTargetWnd                                                     
*  Description    : Get Target Window
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0108
*  Date           : 18th Mar ,2014
****************************************************************************************************/
CWnd* CSystemTray::GetTargetWnd() const
{
    if (m_pTargetWnd)
        return m_pTargetWnd;
    else
        return AfxGetMainWnd();
} // CSystemTray::GetTargetWnd()

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification message stuff

/***************************************************************************************************                    
*  Function Name  : SetCallbackMessage                                                     
*  Description    : Set the Call back function.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0109
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetCallbackMessage(UINT uCallbackMessage)
{
    if (!m_bEnabled)
        return FALSE;

    // Make sure we avoid conflict with other messages
    ASSERT(uCallbackMessage >= WM_APP);

    m_tnd.uCallbackMessage = uCallbackMessage;
    m_tnd.uFlags = NIF_MESSAGE;

    if (m_bHidden)
        return TRUE;
    else
        return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

/***************************************************************************************************                    
*  Function Name  : GetCallbackMessage                                                     
*  Description    : Get the Call back function.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0110
*  Date           : 18th Mar ,2014
****************************************************************************************************/
UINT CSystemTray::GetCallbackMessage() const
{
    return m_tnd.uCallbackMessage;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray menu manipulation

/***************************************************************************************************                    
*  Function Name  : SetMenuDefaultItem                                                     
*  Description    : Set the Default Menu Item
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0111
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::SetMenuDefaultItem(UINT uItem, BOOL bByPos)
{
#ifdef _WIN32_WCE
    return FALSE;
#else
    if ((m_DefaultMenuItemID == uItem) && (m_DefaultMenuItemByPos == bByPos)) 
        return TRUE;

    m_DefaultMenuItemID = uItem;
    m_DefaultMenuItemByPos = bByPos;   

    CMenu menu, *pSubMenu;

    if (!menu.LoadMenu(m_tnd.uID))
        return FALSE;

    pSubMenu = menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

    return TRUE;
#endif
}

/***************************************************************************************************                    
*  Function Name  : GetMenuDefaultItem                                                     
*  Description    : Get the Default Menu Item
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0112
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::GetMenuDefaultItem(UINT& uItem, BOOL& bByPos)
{
    uItem = m_DefaultMenuItemID;
    bByPos = m_DefaultMenuItemByPos;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray message handlers

BEGIN_MESSAGE_MAP(CSystemTray, CWnd)
	//{{AFX_MSG_MAP(CSystemTray)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
#ifndef _WIN32_WCE
	ON_WM_SETTINGCHANGE()
#endif
    ON_REGISTERED_MESSAGE(CSystemTray::m_nTaskbarCreatedMsg, OnTaskbarCreated)
	ON_COMMAND(ID_ITIN_AV, &CSystemTray::OnPopupItinantivirus)
	ON_COMMAND(ID_EMAIL_SCAN, &CSystemTray::OnPopupEmailScan)
	ON_COMMAND(ID_ANTI_ROOTKIT, &CSystemTray::OnPopupAntirootkit)
	ON_COMMAND(ID_AUTOUPDATE, &CSystemTray::OnPopupAutoUpdate)
	ON_COMMAND(ID_USB_DETECT, &CSystemTray::OnPopupUsbdetect)
	ON_COMMAND(ID_TRAY_NOTIFICATION, &CSystemTray::OnPopupTrayNotification)
	ON_COMMAND(ID_STARTUP_SCAN, &CSystemTray::OnPopupStartupScan)
	ON_COMMAND(ID_EXIT, &CSystemTray::OnPopupExit)
	ON_COMMAND(ID_ENABLE_ACTIVE_SCAN, &CSystemTray::OnClickEnableActiveScan)
	ON_COMMAND(ID_DIS_ACTIVE_SCAN_15_MINS, &CSystemTray::OnClickDisableActiveScanFor15Min)
	ON_COMMAND(ID_DIS_ACTIVE_SCAN_1_HR, &CSystemTray::OnClickDisableActiveScanFor1Hr)
	ON_COMMAND(ID_DIS_ACTIVE_SCAN_UNTILL_REBOOT, &CSystemTray::OnClickDisableActiveScanUntillReboot)
	ON_COMMAND(ID_DIS_ACTIVE_SCAN_PERMANENTLY, &CSystemTray::OnClickDisableActiveScanPermanentely)
	
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : OnTimer                                                     
*  Description    : Set Event for timer.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0113
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::OnTimer(UINT_PTR nIDEvent) 
{
    if (nIDEvent != m_uIDTimer)
    {
        ASSERT(FALSE);
        return;
    }

    COleDateTime CurrentTime = COleDateTime::GetCurrentTime();
    COleDateTimeSpan period = CurrentTime - m_StartTime;

    if (m_nAnimationPeriod > 0 && m_nAnimationPeriod < period.GetTotalSeconds())
    {
        StopAnimation();
        return;
    }

    StepAnimation();
}

// This is called whenever the taskbar is created (eg after explorer crashes
// and restarts. Please note that the WM_TASKBARCREATED message is only passed
// to TOP LEVEL windows (like WM_QUERYNEWPALETTE)
/***************************************************************************************************                    
*  Function Name  : OnTaskbarCreated                                                     
*  Description    : Task Bar Creation.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0114
*  Date           : 18th Mar ,2014
****************************************************************************************************/
LRESULT CSystemTray::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
    m_bShowIconPending = TRUE; // !m_bHidden;
    InstallIconPending();

    return 0L;
}

#ifndef _WIN32_WCE

/***************************************************************************************************                    
*  Function Name  : OnSettingChange                                                     
*  Description    : On Change of setting.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0115
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CWnd::OnSettingChange(uFlags, lpszSection);

	__try{
		if (uFlags == SPI_SETWORKAREA)
		{
			m_bShowIconPending = !m_bHidden;
			InstallIconPending();	
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
#endif

/***************************************************************************************************                    
*  Function Name  : OnTrayNotification                                                     
*  Description    : Notifying the tray.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0116
*  Date           : 18th Mar ,2014
****************************************************************************************************/
LRESULT CSystemTray::OnTrayNotification(UINT wParam, LONG lParam) 
{
	CISpyAVTrayDlg* pMainDlg = (CISpyAVTrayDlg*)AfxGetMainWnd();
	if (pMainDlg != NULL)
	{
		pMainDlg->GetProductID();
		CString csProdInfo;
		csProdInfo.Format(L"%s\n", pMainDlg->m_csInstalledEdition);
		if (pMainDlg->CheckProductInfo(csProdInfo) == false)
		{
			pMainDlg->GetRegisteredUserInfo();
			if (theApp.m_dwOsVersion == WINOS_XP || theApp.m_dwOsVersion == WINOS_XP64)
			{
				csProdInfo.AppendFormat(L"%s: %s\n%s: %s",
					theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_PROD_VER"), pMainDlg->m_csRegProductVer,
					theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DB_VER"), pMainDlg->m_csRegDataBaseVer);
			}
			else
			{
				csProdInfo.AppendFormat(L"%s: %s\n%s: %s\n%s: %s", 
					theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_PROD_VER"), pMainDlg->m_csRegProductVer,
					theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DB_VER"), pMainDlg->m_csRegDataBaseVer,
					theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_LAST_UPDATE"), pMainDlg->m_csUpdateDate);
				//theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_THREAT_COUNT"), pMainDlg->m_csThreatDefCount);
			}
		}
		SetTooltipText(csProdInfo);
	}
	//Return quickly if its not for this tray icon
	if (wParam != m_tnd.uID)
        return 0L;

    CMenu menu, *pSubMenu;
    CWnd *pTargetWnd = GetTargetWnd();
    if (!pTargetWnd)
        return 0L;

    // Clicking with right button brings up a context menu
#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    BOOL bAltPressed = ((GetKeyState(VK_MENU) & (1 << (sizeof(SHORT)*8-1))) != 0);
    if (LOWORD(lParam) == WM_LBUTTONUP && bAltPressed)
#else
    if (LOWORD(lParam) == WM_RBUTTONUP)
#endif
    {    

		//call your own CreatePopupMenu function here.
		CPoint pos;
		GetCursorPos(&pos);
//		ContextMenu(pos);
		if(m_sMenu!=NULL)
		{
			m_sMenu = NULL;		
		}
		else
		{
			ContextMenu(pos);
		}
    } 
#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    if (LOWORD(lParam) == WM_LBUTTONDBLCLK && bAltPressed)
#else
    else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) 
#endif
    {
        // double click received, the default action is to execute default menu item
        pTargetWnd->SetForegroundWindow();  

		//On Left Mouse double click need to lauch UI
		LaunchISpyUI();

        UINT uItem;
        if (m_DefaultMenuItemByPos)
        {
            if (!menu.LoadMenu(m_tnd.uID))
                return 0;
            
            pSubMenu = menu.GetSubMenu(0);
            if (!pSubMenu)
                return 0;
            
            uItem = pSubMenu->GetMenuItemID(m_DefaultMenuItemID);

			menu.DestroyMenu();
        }
        else
            uItem = m_DefaultMenuItemID;
        
        pTargetWnd->PostMessage(WM_COMMAND, uItem, 0);
    }


#if defined(_WIN32_WCE) //&& _WIN32_WCE < 211
    if (LOWORD(lParam) == WM_RBUTTONDBLCLK && bAltPressed)
#else
	else if (LOWORD(lParam) == WM_RBUTTONDBLCLK) 
#endif
    {
		CPoint pos;
		GetCursorPos(&pos);
		//ContextMenu(pos);
		if(m_sMenu!=NULL)
		{
			m_sMenu = NULL;
		
		}
		else
		{
			ContextMenu(pos);
		}
	}

    return 1;
}

/***************************************************************************************************                    
*  Function Name  : WindowProc                                                     
*  Description    : Calling Window Procedure.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0117
*  Date           : 18th Mar ,2014
****************************************************************************************************/
LRESULT CSystemTray::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if (message == m_tnd.uCallbackMessage)
	{
		return OnTrayNotification(static_cast<UINT>(wParam), static_cast<LONG>(lParam));
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************                    
*  Function Name  : InstallIconPending                                                     
*  Description    : Install Icon Pending
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0118
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::InstallIconPending()
{
    // Is the icon display pending, and it's not been set as "hidden"?
    if (!m_bShowIconPending || m_bHidden)
        return;

	// Reset the flags to what was used at creation
	m_tnd.uFlags = m_uCreationFlags;

    // Try and recreate the icon
    m_bHidden = !Shell_NotifyIcon(NIM_ADD, &m_tnd);

    // If it's STILL hidden, then have another go next time...
    m_bShowIconPending = m_bHidden;
}

/////////////////////////////////////////////////////////////////////////////
// For minimising/maximising from system tray

/***************************************************************************************************                    
*  Function Name  : FindTrayWnd                                                     
*  Description    : Find Tray Window.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0119
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);

    // Did we find the Main System Tray? If so, then get its size and keep going
    if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        ::GetWindowRect(hwnd, pRect);
        return TRUE;
    }

    // Did we find the System Clock? If so, then adjust the size of the rectangle
    // we have and quit (clock will be found after the system tray)
    if (_tcscmp(szClassName, _T("TrayClockWClass")) == 0)
    {
        CRect *pRect = (CRect*) lParam;
        CRect rectClock;
        ::GetWindowRect(hwnd, rectClock);
        // if clock is above system tray adjust accordingly
        if (rectClock.bottom < pRect->bottom-5) // 10 = random fudge factor.
            pRect->top = rectClock.bottom;
        else
            pRect->right = rectClock.left;
        return FALSE;
    }
 
    return TRUE;
}
 
#ifndef _WIN32_WCE
// enhanced version by Matthew Ellis <m.t.ellis@bigfoot.com>

/***************************************************************************************************                    
*  Function Name  : GetTrayWndRect                                                     
*  Description    : Get Window rect tray.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0120
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::GetTrayWndRect(LPRECT lprect)
{
#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

    HWND hShellTrayWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
    if (hShellTrayWnd)
    {
        ::GetWindowRect(hShellTrayWnd, lprect);
        EnumChildWindows(hShellTrayWnd, FindTrayWnd, (LPARAM)lprect);
        return;
    }
    // OK, we failed to get the rect from the quick hack. Either explorer isn't
    // running or it's a new version of the shell with the window class names
    // changed (how dare Microsoft change these undocumented class names!) So, we
    // try to find out what side of the screen the taskbar is connected to. We
    // know that the system tray is either on the right or the bottom of the
    // taskbar, so we can make a good guess at where to minimize to
    APPBARDATA appBarData;
    appBarData.cbSize=sizeof(appBarData);
    if (SHAppBarMessage(ABM_GETTASKBARPOS,&appBarData))
    {
        // We know the edge the taskbar is connected to, so guess the rect of the
        // system tray. Use various fudge factor to make it look good
        switch(appBarData.uEdge)
        {
        case ABE_LEFT:
        case ABE_RIGHT:
            // We want to minimize to the bottom of the taskbar
            lprect->top    = appBarData.rc.bottom-100;
            lprect->bottom = appBarData.rc.bottom-16;
            lprect->left   = appBarData.rc.left;
            lprect->right  = appBarData.rc.right;
            break;
            
        case ABE_TOP:
        case ABE_BOTTOM:
            // We want to minimize to the right of the taskbar
            lprect->top    = appBarData.rc.top;
            lprect->bottom = appBarData.rc.bottom;
            lprect->left   = appBarData.rc.right-100;
            lprect->right  = appBarData.rc.right-16;
            break;
        }
        return;
    }
    
    // Blimey, we really aren't in luck. It's possible that a third party shell
    // is running instead of explorer. This shell might provide support for the
    // system tray, by providing a Shell_TrayWnd window (which receives the
    // messages for the icons) So, look for a Shell_TrayWnd window and work out
    // the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
    // and stretches either the width or the height of the screen. We can't rely
    // on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
    // rely on it being any size. The best we can do is just blindly use the
    // window rect, perhaps limiting the width and height to, say 150 square.
    // Note that if the 3rd party shell supports the same configuraion as
    // explorer (the icons hosted in NotifyTrayWnd, which is a child window of
    // Shell_TrayWnd), we would already have caught it above
    if (hShellTrayWnd)
    {
        ::GetWindowRect(hShellTrayWnd, lprect);
        if (lprect->right - lprect->left > DEFAULT_RECT_WIDTH)
            lprect->left = lprect->right - DEFAULT_RECT_WIDTH;
        if (lprect->bottom - lprect->top > DEFAULT_RECT_HEIGHT)
            lprect->top = lprect->bottom - DEFAULT_RECT_HEIGHT;
        
        return;
    }
    
    // OK. Haven't found a thing. Provide a default rect based on the current work
    // area
    SystemParametersInfo(SPI_GETWORKAREA,0, lprect, 0);
    lprect->left = lprect->right - DEFAULT_RECT_WIDTH;
    lprect->top  = lprect->bottom - DEFAULT_RECT_HEIGHT;
}

// Check to see if the animation has been disabled (Matthew Ellis <m.t.ellis@bigfoot.com>)

/***************************************************************************************************                    
*  Function Name  : GetDoWndAnimation                                                     
*  Description    : Get the Windw Animation.
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0121
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::GetDoWndAnimation()
{
  ANIMATIONINFO ai;

  ai.cbSize=sizeof(ai);
  SystemParametersInfo(SPI_GETANIMATION,sizeof(ai),&ai,0);

  return ai.iMinAnimate?TRUE:FALSE;
}
#endif

/***************************************************************************************************                    
*  Function Name  : RemoveTaskbarIcon                                                     
*  Description    : Remove Task bar Icon
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0122
*  Date           : 18th Mar ,2014
****************************************************************************************************/
BOOL CSystemTray::RemoveTaskbarIcon(CWnd* pWnd)
{
    LPCTSTR pstrOwnerClass = AfxRegisterWndClass(0);

    // Create static invisible window
    if (!::IsWindow(m_wndInvisible.m_hWnd))
    {
		if (!m_wndInvisible.CreateEx(0, pstrOwnerClass, _T(""), WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, 0))
			return FALSE;
    }

    pWnd->SetParent(&m_wndInvisible);

    return TRUE;
}

/***************************************************************************************************                    
*  Function Name  : MinimiseToTray                                                     
*  Description    : Minimise Tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0123
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::MinimiseToTray(CWnd* pWnd, BOOL bForceAnimation /*= FALSE*/)
{
#ifndef _WIN32_WCE
    if (bForceAnimation || GetDoWndAnimation())
    {
        CRect rectFrom, rectTo;

        pWnd->GetWindowRect(rectFrom);
        GetTrayWndRect(rectTo);

        ::DrawAnimatedRects(pWnd->m_hWnd, IDANI_CAPTION, rectFrom, rectTo);
    }

    RemoveTaskbarIcon(pWnd);
    pWnd->ModifyStyle(WS_VISIBLE, 0);
#endif
}

/***************************************************************************************************                    
*  Function Name  : MaximiseFromTray                                                     
*  Description    : Maximise Tray
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0124
*  Date           : 18th Mar ,2014
****************************************************************************************************/
void CSystemTray::MaximiseFromTray(CWnd* pWnd, BOOL bForceAnimation /*= TRUE*/)
{
#ifndef _WIN32_WCE
    if (bForceAnimation || GetDoWndAnimation())
    {
        CRect rectTo;
        pWnd->GetWindowRect(rectTo);

        CRect rectFrom;
        GetTrayWndRect(rectFrom);

        pWnd->SetParent(NULL);
        ::DrawAnimatedRects(pWnd->m_hWnd, IDANI_CAPTION, rectFrom, rectTo);
    }
    else
        pWnd->SetParent(NULL);

    pWnd->ModifyStyle(0, WS_VISIBLE);
    pWnd->RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME |
                                   RDW_INVALIDATE | RDW_ERASE);

    // Move focus away and back again to ensure taskbar icon is recreated
    if (::IsWindow(m_wndInvisible.m_hWnd))
        m_wndInvisible.SetActiveWindow();
    pWnd->SetActiveWindow();
    pWnd->SetForegroundWindow();
#endif
}

/***************************************************************************************************                    
*  Function Name  : LaunchISpyUI                                                     
*  Description    : Launch WardWiz UI
*  Author Name    : Nitin
*  SR_NO		  : WRDWIZTRAY_0125
*  Date           : 18th Mar ,2014
****************************************************************************************************/
bool CSystemTray::LaunchISpyUI()
{
	CString csUIFilePath;
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		return false;
	}
	csUIFilePath = szModulePath;
	csUIFilePath += L"\\VBUI.EXE";

	ShellExecute(NULL, L"open", csUIFilePath, NULL, NULL, SW_SHOW);
	return true;
}

/***********************************************************************************************
  Function Name  : CheckMenuItem
  Description    : This method is used to checking the Menu Item State Checked/Unchacked
  Author Name    : Nitin Kolapkar
  SR_NO		  : WRDWIZTRAY_0126
  Date           : 6th May 2014
***********************************************************************************************/
BOOL CSystemTray::CheckMenuItem(UINT uiCmd, BOOL bCheck)
{
    CString sMenuString;
    CMenu *pSubMenu;

    pSubMenu = m_Menu.GetSubMenu(0);
    if (!pSubMenu)
        return FALSE;

    pSubMenu->GetMenuString(uiCmd, sMenuString, MF_BYCOMMAND);
    if (bCheck)
        pSubMenu->ModifyMenu(uiCmd, 
                             MF_CHECKED | MF_BYCOMMAND, 
                             uiCmd,
                             sMenuString);
    else
        pSubMenu->ModifyMenu(uiCmd, 
                             MF_UNCHECKED | MF_BYCOMMAND , 
                             uiCmd,
                             sMenuString);
    return TRUE;
} // CSystemTray::CheckMenuItem()


/***********************************************************************************************
  Function Name  : ContextMenu
  Description    : This method is used to Launching Menu Item On Right Click Of iTIN AV ICON
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		  : WRDWIZTRAY_0127
***********************************************************************************************/
void CSystemTray::ContextMenu(CPoint point) 
{
	try
	{
		CMenu  *pobjTrayMenu = new CMenu();
		if(!pobjTrayMenu)
		{
			AddLogEntry(L"### Exception in CSystemTray::ContextMenu::pobjTrayMenu", 0, 0, true, SECONDLEVEL);
			return;
		}

		SetRegistryFlag();

		UINT uFlag = MF_BYPOSITION | MF_STRING;

		pobjTrayMenu->CreatePopupMenu();
		pobjTrayMenu->AppendMenuW(uFlag, ID_ITIN_AV,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"));
		pobjTrayMenu->SetDefaultItem(ID_ITIN_AV);
		pobjTrayMenu->AppendMenuW(MF_SEPARATOR, NULL, L"-");

		CString m_stMenuItem;
		UINT m_DisableFlag = 0;
		m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();

		//Update here the Number of days left.
		theApp.GetDaysLeft();
		//Issue No 658 If no of days  =0  all items in system menu should be disable.
		// Neha Gharge 3 July,2015
		if(GetExistingPathofOutlook() && theApp.m_dwDaysLeft > 0)
		{
			m_bOutlookFlag = true;
			m_DisableFlag = 0;
		}
		else
		{
			m_bOutlookFlag = false;
			m_DisableFlag = MF_GRAYED;
		}

		if(theApp.m_dwProductID != ESSENTIAL && theApp.m_dwProductID != BASIC)
		{
			//hide Email Scan in right click tray
			/*m_stMenuItem = theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_EMAILSCAN_TEXT");
			pobjTrayMenu->AppendMenuW(uFlag | m_DisableFlag, ID_EMAIL_SCAN, m_stMenuItem);
			if(m_bEmailScan)
				pobjTrayMenu->CheckMenuItem(ID_EMAIL_SCAN,MF_CHECKED);
			else
				pobjTrayMenu->CheckMenuItem(ID_EMAIL_SCAN,MF_UNCHECKED);*/
		}

		m_DisableFlag = 0;
		/* Nitin K 25 June 2014 AV Tray Functionalities */
		m_stMenuItem = theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_TRAY_NOTIFICATION");
		pobjTrayMenu->AppendMenuW(uFlag | m_DisableFlag, ID_TRAY_NOTIFICATION, m_stMenuItem);

		if(m_bTrayNotification)
			pobjTrayMenu->CheckMenuItem(ID_TRAY_NOTIFICATION,MF_CHECKED);
		else
			pobjTrayMenu->CheckMenuItem(ID_TRAY_NOTIFICATION,MF_UNCHECKED);

		m_DisableFlag = 0;
		m_stMenuItem = theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_STARTUP_SCAN");
		pobjTrayMenu->AppendMenuW(uFlag | m_DisableFlag, ID_STARTUP_SCAN, m_stMenuItem);

		if(m_bStartUpScan)
			pobjTrayMenu->CheckMenuItem(ID_STARTUP_SCAN,MF_CHECKED);
		else
			pobjTrayMenu->CheckMenuItem(ID_STARTUP_SCAN,MF_UNCHECKED);


		if (theApp.m_dwDaysLeft > 0)
		{
			m_DisableFlag = 0;
		}

		//Ram:
		//Removed as because User can able to do update if product is un-registered.
		//else
		//{
		//	m_DisableFlag = MF_GRAYED;
		//}
		m_stMenuItem = theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_AUTOLIVEUPDATE_TEXT");
		pobjTrayMenu->AppendMenuW(uFlag | m_DisableFlag, ID_AUTOUPDATE, m_stMenuItem);

		if(m_bAutoUpdate)
			pobjTrayMenu->CheckMenuItem(ID_AUTOUPDATE,MF_CHECKED);
		else
			pobjTrayMenu->CheckMenuItem(ID_AUTOUPDATE,MF_UNCHECKED);


		m_DisableFlag = 0;
		m_stMenuItem = theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_USBSCAN_TEXT");
		pobjTrayMenu->AppendMenuW(uFlag | m_DisableFlag, ID_USB_DETECT, m_stMenuItem);

		if(m_bUsbDetect)
			pobjTrayMenu->CheckMenuItem(ID_USB_DETECT,MF_CHECKED);
		else
			pobjTrayMenu->CheckMenuItem(ID_USB_DETECT,MF_UNCHECKED);

		//pobjTrayMenu->AppendMenuW(MF_SEPARATOR, NULL, L"-");
		//m_stMenuItem = theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_EXIT_TEXT");
		//pobjTrayMenu->AppendMenuW(uFlag , ID_EXIT, m_stMenuItem);

		pobjTrayMenu->AppendMenuW(MF_SEPARATOR, NULL, L"-");

		CMenu *ActiveScanMenu = new CMenu;
		if (!ActiveScanMenu)
		{
			AddLogEntry(L"### Exception in CSystemTray::ContextMenu::ActiveScanMenu", 0, 0, true, SECONDLEVEL);
			return;
		}
		ActiveScanMenu->CreatePopupMenu();
		if (!m_bActiveScanOption)
		{
			ActiveScanMenu->AppendMenuW(uFlag | MF_GRAYED, ID_ENABLE_ACTIVE_SCAN, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_ENABLE_ACTIVE_SCAN"));
		}
		if (m_bActiveScanOption)
		{
			ActiveScanMenu->AppendMenuW(uFlag, ID_ENABLE_ACTIVE_SCAN, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_ENABLE_ACTIVE_SCAN"));
		}
		ActiveScanMenu->AppendMenuW(MF_SEPARATOR, NULL, L"-");

		ActiveScanMenu->AppendMenuW(uFlag, ID_DIS_ACTIVE_SCAN_15_MINS, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DISABLE_ACTIVE_SCAN_FIFTEEN_MIN"));
		ActiveScanMenu->AppendMenuW(uFlag, ID_DIS_ACTIVE_SCAN_1_HR, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DISABLE_ACTIVE_SCAN_ONE_HR"));
		ActiveScanMenu->AppendMenuW(uFlag, ID_DIS_ACTIVE_SCAN_UNTILL_REBOOT, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DISABLE_ACTIVE_SCAN_UNTILL_RESTART"));
		ActiveScanMenu->AppendMenuW(uFlag, ID_DIS_ACTIVE_SCAN_PERMANENTLY, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_DISABLE_ACTIVE_SCAN_PERMANENT"));

		pobjTrayMenu->AppendMenuW(MF_POPUP, (UINT_PTR)ActiveScanMenu->m_hMenu, theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_ACTIVE_SCAN"));
		uFlag = MF_BYPOSITION | MF_STRING;

		CWnd *pWnd = AfxGetMainWnd();
		pWnd->SetForegroundWindow();
		pWnd->PostMessage(WM_NULL, 0, 0);
		pobjTrayMenu->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this);

		if(pobjTrayMenu != NULL)
		{
			delete pobjTrayMenu;
			pobjTrayMenu = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::ContextMenu", 0, 0, true, SECONDLEVEL);
	}
}



/***********************************************************************************************
  Function Name  : OnPopupItinantivirus
  Description    : This method is used to Launching application from Menu ITEM
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0128
***********************************************************************************************/
void CSystemTray::OnPopupItinantivirus()
{

	LaunchISpyUI();	
}

/***********************************************************************************************
  Function Name  : OnPopupEmailScan
  Description    : This method is used to set CHECK/UNCHECK flag for MENU ITEM
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0129
***********************************************************************************************/
void CSystemTray::OnPopupEmailScan()
{
	//ISSue no :33 neha Gharge 31/5/2014 **********************/	
	DWORD flag= ReadRegistryEntryofMenuItem(L"dwEmailScan");
	if(flag==0)
	{
		//After enabling and disabling Email scan all sub option getting updated
		//Neha Gharge 12 Aug,2015
		flag=1;
		if(WriteRegistryEntryOfMenuItem(L"dwEmailScan",flag))
		{
			//OnPopupEmailScanSubOptions(flag);
			m_bEmailScan=TRUE;
		}
		if(!SendEmailPluginChange2EmailPlugin(ENABLE_DISABLE_EMAIL_PLUGIN,1))
		{
			AddLogEntry(L"Error in CSystemTray::SendEmailPluginChange2EmailPlugin", 0, 0, true, SECONDLEVEL);
		}
	}
	else
	{
		flag=0;
		if(WriteRegistryEntryOfMenuItem(L"dwEmailScan",flag))
		{
			//OnPopupEmailScanSubOptions(flag);
			m_bEmailScan=FALSE;
		}
		if(!SendEmailPluginChange2EmailPlugin(ENABLE_DISABLE_EMAIL_PLUGIN,0))
		{
			AddLogEntry(L"Error in CSystemTray::SendEmailPluginChange2EmailPlugin", 0, 0, true, SECONDLEVEL);
		}
	}

/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
	SendEnableEmailScanMessage2UI(eEMAIL);

}



/***********************************************************************************************
  Function Name  : SendEnableEmailScanMessage2UI
  Description    : Send Enable/Disable email Scan Status to UI
  Author Name    : Neha Gharge
  Date           : 10th May 2014
  SR_NO		     : WRDWIZTRAY_0130
***********************************************************************************************/
bool SendEnableEmailScanMessage2UI(DWORD dwSelectedOption)
{
	__try
	{
		
		HWND hWindow = FindWindow( NULL,L"WRDWIZAVUI");
		SendMessage(hWindow, CHECKEMAILSCANSTATUS, dwSelectedOption, 0);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CSystemTray::SendEnableEmailScanMessage2UI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***********************************************************************************************
  Function Name  : OnPopupEmailScanSubOptions
  Description    : This method is used to set registry values for Email Scan options
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0131
***********************************************************************************************/
void CSystemTray::OnPopupEmailScanSubOptions(DWORD FlagValue)
{
	if(m_bOutlookFlag)
	{
		LPCTSTR emailScanOptionsPath = theApp.m_csRegKeyPath + TEXT("\\EmailScanSetting");
		LPCTSTR settingsTabPath = theApp.m_csRegKeyPath;
		CStringArray emailScanOptions;
		emailScanOptions.Add(L"dwEnableContentFilter");
		emailScanOptions.Add(L"dwEnableSignature");
		emailScanOptions.Add(L"dwEnableSpamFilter");
		emailScanOptions.Add(L"dwEnableVirusScan");

		for(int i =0; i<emailScanOptions.GetSize();i++)
		{
			if(!WriteRegistryEntryOfEmailScanSubOptions(emailScanOptionsPath,(CString)emailScanOptions.GetAt(i),FlagValue))
			{
				AddLogEntry(L"### Error in AVTray Registry CSystemTray::OnPopupEmailScanSubOptions", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	else
	{
		MessageBoxW(theApp.m_objwardwizLangManager.GetString(L"IDS_INSTALL_OUTLOOK"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	}

}

/***********************************************************************************************
  Function Name  : WriteRegistryEntryOfEmailScanSubOptions
  Description    : this function is  called for Writing the Registry Key Value for Tray option Email Scan
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0132
***********************************************************************************************/
BOOL CSystemTray::WriteRegistryEntryOfEmailScanSubOptions(LPCTSTR SubKey,CString strKey, DWORD dwChangeValue)
{
	if(!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue,true))
	{
		AddLogEntry(L"### Error in Setting Registry CHTMLListCtrl::SettingsTabEmailScan", 0, 0, true, SECONDLEVEL);
	}
	Sleep(40);
	return TRUE;
}

/***********************************************************************************************
  Function Name  : OnPopupAntirootkit
  Description    : this function is  called for setting CHECK/UNCHECK flag for MENU ITEM
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0133
***********************************************************************************************/
void CSystemTray::OnPopupAntirootkit()
{
	DWORD flag= ReadRegistryEntryofMenuItem(L"dwAntirootkit");
	if(flag==0)
	{
		if(WriteRegistryEntryOfMenuItem(L"dwAntirootkit",1))
		{
			m_bAntiRootkit=TRUE;
		}
	}
	else
	{
		
		if(WriteRegistryEntryOfMenuItem(L"dwAntirootkit",0))
		{
			m_bAntiRootkit=FALSE;
		}
	}


}


/***********************************************************************************************
  Function Name  : OnPopupAutoUpdate
  Description    : this function is  called for setting CHECK/UNCHECK flag for MENU ITEM
  Author Name    : Nitin Kolapkar
  Date           : 29th May 2014
  SR_NO		     : WRDWIZTRAY_0134
***********************************************************************************************/
void CSystemTray::OnPopupAutoUpdate()
{
	/* its commented due to resloving coflict between UI  and system Tray  and  "Auto Product Update " menu from setting  tab is also remove  */

	/*DWORD flag= ReadRegistryEntryofMenuItem(L"dwAutoDefUpdate");
	if(flag==0)
	{
		if(WriteRegistryEntryOfMenuItem(L"dwAutoDefUpdate",1) && WriteRegistryEntryOfMenuItem(L"dwAutoProductUpdate",1))
		{
			m_bAutoUpdate=TRUE;
		}
	}
	else
	{
		
		if(WriteRegistryEntryOfMenuItem(L"dwAutoDefUpdate",0) && WriteRegistryEntryOfMenuItem(L"dwAutoProductUpdate",0))
		{
			m_bAutoUpdate=FALSE;
		}
	}*/
/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */

	DWORD flag= ReadRegistryEntryofMenuItem(L"dwAutoDefUpdate");
	if(flag==0)
	{
		if(WriteRegistryEntryOfMenuItem(L"dwAutoDefUpdate",1))
		{
			m_bAutoUpdate=TRUE;
		}
	}
	else
	{
		
		if(WriteRegistryEntryOfMenuItem(L"dwAutoDefUpdate",0))
		{
			m_bAutoUpdate=FALSE;
		}
		//issue no 4: Autolive get notification as soon as we off check box from tray
		SendRequestALUservToStopUpdate(STOP_UPDATE);
	}

	/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */	
	SendEnableEmailScanMessage2UI(eAUTOLIVESCAN);

}
/***********************************************************************************************
  Function Name  : OnPopupUsbdetect
  Description    : this function is  called for setting CHECK/UNCHECK flag for MENU ITEM
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0135
***********************************************************************************************/
void CSystemTray::OnPopupUsbdetect()
{
	DWORD flag= ReadRegistryEntryofMenuItem(L"dwUsbScan");
	if(flag==0)
	{
		if(WriteRegistryEntryOfMenuItem(L"dwUsbScan",1))
		{
			m_bUsbDetect=TRUE;
		}
	}
	else
	{
		if(WriteRegistryEntryOfMenuItem(L"dwUsbScan",0))
		{
			m_bUsbDetect=FALSE;
		}
	}

 /*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
	SendEnableEmailScanMessage2UI(eUSBDECTION);
}
/***********************************************************************************************
  Function Name  : OnPopupExit
  Description    : exits the tray application
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  Isuue Number - 295 Rajil Y. 22/05/2014
  SR_NO		     : WRDWIZTRAY_0136
***********************************************************************************************/
void CSystemTray::OnPopupExit()
{
	//if ( //MessageBox(m_hWnd,theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_TRAY_EXIT_TEXT"),
	//	MessageBoxA(m_hWnd,theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_TRAY_EXIT_TEXT"),
	//	theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
	if(m_bExitPopupflag == true)
		return;
	m_bExitPopupflag = true;
	if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_AVTRAY_TRAY_EXIT_TEXT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION|MB_TOPMOST | MB_YESNO) == IDYES)
	{
		m_bExitPopupflag = false;
		if(SendData2UI(EXIT_APPLICATION,  true))
		{	// resolved by lalit, tool tip window not getting close when exit from tray exit 
			  closeToolTipWindow();
			::SendMessage(AfxGetMainWnd()->m_hWnd, WM_CLOSE, 0, 0);
		}
	}
	else
	{
		m_bExitPopupflag = false;
	}
}

/***********************************************************************************************
  Function Name  : ReadRegistryEntryofMenuItem
  Description    : this function is  called for Reading values from Registry for Anti Rootkit, Email Scan, USB Detect
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0137
***********************************************************************************************/
DWORD CSystemTray::ReadRegistryEntryofMenuItem(CString strKey)
{
	DWORD dwType = REG_DWORD;
	DWORD returnDWORDValue;
	DWORD dwSize = sizeof(returnDWORDValue);
	HKEY hKey;
	returnDWORDValue = 0;
	LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, LPCTSTR(m_Key), 0,KEY_READ, &hKey);
	if(lResult == ERROR_SUCCESS)
		lResult = ::RegQueryValueEx(hKey, strKey, NULL,&dwType, (LPBYTE)&returnDWORDValue, &dwSize);
		if(lResult == ERROR_SUCCESS)
		{	
			::RegCloseKey(hKey);
			return returnDWORDValue;	
		}
		else 
		{
			::RegCloseKey(hKey);
			return 0;
		}

	return 0;
}

/***********************************************************************************************
  Function Name  : WriteRegistryEntryOfMenuItem
  Description    : this function is  called for Write the Registry Key Value for Menu Items
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0138
***********************************************************************************************/
BOOL CSystemTray::WriteRegistryEntryOfMenuItem(CString strKey, DWORD dwChangeValue)
{
	LPCTSTR SubKey = theApp.m_csRegKeyPath;
	if(!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue,true))
	{
		AddLogEntry(L"### Error in Setting Registry CSystemTray::MenuItem", 0, 0, true, SECONDLEVEL);
	}
	Sleep(50);
	return TRUE;
}


/***********************************************************************************************
  Function Name  : SetRegistrykeyUsingService
  Description    : this function is  called for Set the Registry Key Value for Menu Items using Service
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0139
***********************************************************************************************/
bool CSystemTray::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD; 
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName );
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to set data in CSystemTray : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CSystemTray : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}


/***********************************************************************************************
  Function Name  : SetRegistryFlag
  Description    : This method is used to set the flag of Menu Items. Values for flags are fetched from Registry.
  Author Name    : Nitin Kolapkar
  Date           : 6th May 2014
  SR_NO		     : WRDWIZTRAY_0140
***********************************************************************************************/
void CSystemTray :: SetRegistryFlag()
{
	DWORD Flag;
	
	Flag= ReadRegistryEntryofMenuItem(L"dwEmailScan");
	if(Flag==0)
	{
		m_bEmailScan=FALSE;
	}
	else
	{
		m_bEmailScan=TRUE;
	}
	
	/*	ISSUE NO - 627 NAME - NITIN K. TIME - 9th June 2014 */
	//Flag= ReadRegistryEntryofMenuItem(L"dwAutoDefUpdate");
	//if(Flag == 0)
//	{
		Flag= ReadRegistryEntryofMenuItem(L"dwAutoDefUpdate");
	//}
	if(Flag==0)
	{
		m_bAutoUpdate=FALSE;
	}
	else
	{
		m_bAutoUpdate=TRUE;
	}
	Flag= ReadRegistryEntryofMenuItem(L"dwUsbScan");
	if(Flag==0)
	{
		m_bUsbDetect=FALSE;
	}
	else
	{
		m_bUsbDetect=TRUE;
	}
	/* Nitin K 25 June 2014 AV Tray Functionalities */
	Flag= ReadRegistryEntryofMenuItem(L"dwShowTrayPopup");
	if(Flag==0)
	{
		m_bTrayNotification = FALSE;
	}
	else
	{
		m_bTrayNotification = TRUE;
	}
	Flag= ReadRegistryEntryofMenuItem(L"dwStartUpScan");
	if(Flag==0)
	{
		m_bStartUpScan=FALSE;
	}
	else
	{
		m_bStartUpScan=TRUE;
	}
	Flag = ReadRegistryEntryofMenuItem(L"dwActiveScanOption");
	if (Flag != 1)
	{
		m_bActiveScanOption = TRUE;
	}
	else
	{
		m_bActiveScanOption = FALSE;
	}
}

/***********************************************************************************************
  Function Name  : SendEmailPluginChange2EmailPlugin()
  Description    : Send message to emailplugin 
  Author Name    : Neha gharge
  Date           : 31/5/2014
  SR_NO		     : WRDWIZTRAY_0141
***********************************************************************************************/
bool CSystemTray::SendEmailPluginChange2EmailPlugin(int iMessageInfo, DWORD dwType)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = iMessageInfo;
	szPipeData.dwSecondValue = dwType;
	
	CISpyCommunicator objCom(EMAILPLUGIN_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CSystemTray::SendEmailPluginChange2EmailPlugin", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
  Function Name  : GetExistingPathofOutlook()
  Description    : Get Existing Path of Outlook
  Author Name    : Neha gharge
  Date           : 31/5/2014
  SR_NO		     : WRDWIZTRAY_0142
***********************************************************************************************/
bool CSystemTray::GetExistingPathofOutlook()
{
	bool bReturn = false;
	try
	{
		//if product is not PRO then no need to check for this.
		if (theApp.m_dwProductID != PRO)
			return false;

		LPTSTR ppszVersion;
		BOOL	pf64Bit = FALSE;
		HRESULT result;
		CString csActualGUIDs;
		CString csInstalledGUID;
		int iPos =0;
		result = GetOutlookVersionString(&ppszVersion,&pf64Bit);

		///if failed to retrive values means outlook is not installed
		if(FAILED(result))
		{
			AddLogEntry(L">>> EmailScanDlg : GetOutlookVersionString:outlook is not installed", 0, 0, true, SECONDLEVEL);
			// only for GetOutlookVersionString get failed for 2016
			// Hence It is remain disable. 
			if (CheckForOutllookExe())
			{
				return true;
			}
			return false;
		}

		for(int i=0 ; i<10 ; i++)
		{
			csActualGUIDs = g_GUIDofInstalledOutllookH[i];
			csInstalledGUID = (LPCTSTR)ppszVersion;
			csInstalledGUID = csInstalledGUID.Tokenize(_T("."),iPos);
			iPos =0;
			if(!(csActualGUIDs.Compare(csInstalledGUID)))
			{
				bReturn = true;
				//if(bReturn)
				//{
				//	//if(pf64Bit == 1)
				//	//{
				//	//	bReturn = false;
				//	//	break;
				//	//}
				//	break;
				//}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanDlg::GetExistingPathofOutlook", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
  Function Name  : GetOutlookVersionString()
  Description    : Get Outlook Version String
  Author Name    : Neha gharge
  Date           : 31/5/2014
  SR_NO		     : WRDWIZTRAY_0143
***********************************************************************************************/
HRESULT CSystemTray::GetOutlookVersionString(LPTSTR* ppszVer, BOOL* pf64Bit)
{
    HRESULT hr = E_FAIL;
	LPTSTR pszTempPath = NULL;
	LPTSTR pszTempVer = NULL;

	__try
	{

		TCHAR pszOutlookQualifiedComponents[][MAX_PATH] = {
			TEXT("{E83B4360-C208-4325-9504-0D23003A74A5}"), // Outlook 2013
			TEXT("{1E77DE88-BCAB-4C37-B9E5-073AF52DFD7A}"), // Outlook 2010
			TEXT("{24AAE126-0911-478F-A019-07B875EB9996}"), // Outlook 2007
			TEXT("{BC174BAD-2F53-4855-A1D5-0D575C19B1EA}")  // Outlook 2003
		};
		int nOutlookQualifiedComponents = _countof(pszOutlookQualifiedComponents);
		int i = 0;
		DWORD dwValueBuf = 0;
		UINT ret = 0;

		*pf64Bit = FALSE;

		for (i = 0; i < nOutlookQualifiedComponents; i++)
		{
			ret = MsiProvideQualifiedComponent(
				pszOutlookQualifiedComponents[i],
				TEXT("outlook.x64.exe"),
				(DWORD) INSTALLMODE_DEFAULT,
				NULL,
				&dwValueBuf);
			if (ERROR_SUCCESS == ret) break;
		}

		if (ret != ERROR_SUCCESS)
		{
			for (i = 0; i < nOutlookQualifiedComponents; i++)
			{
				ret = MsiProvideQualifiedComponent(
					pszOutlookQualifiedComponents[i],
					TEXT("outlook.exe"),
					(DWORD) INSTALLMODE_DEFAULT,
					NULL,
					&dwValueBuf);
				if (ERROR_SUCCESS == ret) break;
			}
		}
		else
		{
			*pf64Bit = TRUE;
		}

		if (ret == ERROR_SUCCESS)
		{
			dwValueBuf += 1;
			pszTempPath = (LPTSTR) malloc(dwValueBuf * sizeof(TCHAR));
			if (pszTempPath != NULL)
			{
				if ((ret = MsiProvideQualifiedComponent(
					pszOutlookQualifiedComponents[i],
					TEXT("outlook.exe"),
					(DWORD) INSTALLMODE_EXISTING,
					pszTempPath,
					&dwValueBuf)) != ERROR_SUCCESS)
				{
					goto Error;
				}

				pszTempVer = (LPTSTR) malloc(MAX_PATH * sizeof(TCHAR));
				dwValueBuf = MAX_PATH;
				if ((ret = MsiGetFileVersion(pszTempPath,
					pszTempVer,
					&dwValueBuf,
					NULL,
					NULL))!= ERROR_SUCCESS)
				{
					goto Error;    
				}
				*ppszVer = pszTempVer;
				pszTempVer = NULL;
				hr = S_OK;
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanDlg::GetOutlookVersionString", 0, 0, true, SECONDLEVEL);
	}

Error:
	if(pszTempVer)
	{
		free(pszTempVer);
	}
	if(pszTempPath)
	{
	    free(pszTempPath);
	}
	return hr;
}

/* Nitin K 25 June 2014 AV Tray Functionalities */
/***********************************************************************************************
  Function Name  : OnPopupTrayNotification
  Description    : this function is  called for setting CHECK/UNCHECK flag for MENU ITEM
  Author Name    : Nitin Kolapkar
  Date           : 25th June 2014
  SR_NO		     : WRDWIZTRAY_0144
***********************************************************************************************/
void CSystemTray::OnPopupTrayNotification()
{
	DWORD flag= ReadRegistryEntryofMenuItem(L"dwShowTrayPopup");
	if(flag==0)
	{
		if(WriteRegistryEntryOfMenuItem(L"dwShowTrayPopup",1))
		{
			theApp.m_dwShowTrayPopup = 1;
			m_bTrayNotification = TRUE;
		}
	}
	else
	{
		if(WriteRegistryEntryOfMenuItem(L"dwShowTrayPopup",0))
		{
			theApp.m_dwShowTrayPopup = 0;
			m_bTrayNotification = FALSE;
		}
	}

	/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
	SendEnableEmailScanMessage2UI(eTRAYNOTIFICATION);
}

/* Nitin K 25 June 2014 AV Tray Functionalities */
/***********************************************************************************************
  Function Name  : OnPopupStartupScan
  Description    : this function is  called for setting CHECK/UNCHECK flag for MENU ITEM
  Author Name    : Nitin Kolapkar
  Date           : 25th June 2014
  SR_NO		     : WRDWIZTRAY_0145
***********************************************************************************************/
void CSystemTray::OnPopupStartupScan()
{
	DWORD flag= ReadRegistryEntryofMenuItem(L"dwStartUpScan");
	if(flag==0)
	{
		if(WriteRegistryEntryOfMenuItem(L"dwStartUpScan",1))
		{
			m_bStartUpScan = TRUE;
		}
	}
	else
	{
		if(WriteRegistryEntryOfMenuItem(L"dwStartUpScan",0))
		{
			m_bStartUpScan = FALSE;
		}
	}
	
   /*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
	SendEnableEmailScanMessage2UI(eSTARTUPSCAN);
}

/***********************************************************************************************
  Function Name  : SendData2UI
  Description    : Send Data to UI
  Author Name    : Nitin Kolapkar
  Date           : 25th June 2014
  SR_NO		     : WRDWIZTRAY_0146
***********************************************************************************************/
bool CSystemTray::SendData2UI(int iMessageInfo, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = iMessageInfo;

	bool bUIExists = true;
	CISpyCommunicator objComUI(UI_SERVER);
	if(!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		bUIExists = false;
		AddLogEntry(L"### Failed to send Data in CSystemTray::SendData2UI", 0, 0, true, SECONDLEVEL);
	}

	if(bWait && bUIExists)
	{
		if(!objComUI.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CSystemTray::SendData2UI", 0, 0, true, SECONDLEVEL);
		}
		if(szPipeData.dwValue == 0)
		{
			return false;
		}
	}
	return true;
}


/***********************************************************************************************                    
*  Function Name  : SendRequestCommon                                                     
*  Description    : Send request to auto live update services.
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 15- January -2015
*************************************************************************************************/
bool CSystemTray::SendRequestALUservToStopUpdate(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue = 2;

		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, false);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to AUTOUPDATESRV_SERVER in CSystemTray::SendRequestALUservToStopUpdate", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CSystemTray::SendRequestALUservToStopUpdate", 0, 0, true, SECONDLEVEL);
	}
	return true;
}
/***********************************************************************************************
*  Function Name  : closeToolTipWindow
*  Description    : it use to close ToolTipWindow when Tray Exit Get Called.
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 5-4-2015
*************************************************************************************************/
void closeToolTipWindow(void)
{
	try
	{
		HWND hWindow = FindWindow(NULL, L"WRDWIZAVUI");
		if (hWindow != NULL)
		{
			SendMessage(hWindow, WM_CLOSE, 0, 0);
			CloseHandle(hWindow);
			hWindow = NULL;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::closeToolTipWindow", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	CheckForOutllookExe()
*  Description    : It will check outlook exe default path.
*  SR.NO          :
*  Author Name    : Neha Gharge
*  Date           : 27th Aug, 2015
*********************************************************************************************************/
bool CSystemTray::CheckForOutllookExe()
{
	try
	{
		HKEY hKey;
		LONG ReadReg;
		TCHAR  szOutlookPath[1024];
		DWORD dwvalueSize = 1024;
		DWORD dwType = REG_SZ;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE"), &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Unable to open registry key outlook exe", 0, 0, true, SECONDLEVEL);
			return false;
		}

		ReadReg = RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)&szOutlookPath, &dwvalueSize);
		if (ReadReg == ERROR_SUCCESS)
		{
			AddLogEntry(L">>> outlook default path %s", szOutlookPath, 0, true, FIRSTLEVEL);
			if (_tcscmp((LPCTSTR)szOutlookPath, L"") == 0)
			{
				RegCloseKey(hKey);
				return false;
			}
			else
			{
				RegCloseKey(hKey);
				return true;
			}
		}
		else
		{
			RegCloseKey(hKey);
			return false;
		}

		RegCloseKey(hKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanDlg::CheckForOutllookExe()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : OnClickEnableActiveScan
Description    : Enables active scan feature.
Author Name    : Nitin Kolapkar
Date           : 13th July 2016
SR_NO		   : 
***********************************************************************************************/
void CSystemTray::OnClickEnableActiveScan()
{
	try
	{
		if (WriteRegistryEntryOfMenuItem(L"dwActiveScanOption", 1))
		{
			m_bActiveScanOption = FALSE;
		}
		if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
		{
			AddLogEntry(L"### Failed to SendData to UI CSystemTray::OnClickEnableActiveScan", 0, 0, true, SECONDLEVEL);
		}
		if (!SendData2Service(HANDLEACTICESCANSETTINGS, ENABLEACTSCAN, 0, 0, 0, true))
		{
			AddLogEntry(L"### Failed to SendData to Service", 0, 0, true, SECONDLEVEL);
		}
		if (!DisplayWidgetsState(GREENFLAG))
		{
			AddLogEntry(L"### Failed DisplayWidgetsState REDFLAG in CSystemTray::OnClickEnableActiveScan", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::OnClickEnableActiveScan", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnClickEnableActiveScan
Description    : Disables active scan feature for 15 mins.
Author Name    : Nitin Kolapkar
Date           : 13th July 2016
SR_NO		   :
***********************************************************************************************/
void CSystemTray::OnClickDisableActiveScanFor15Min()
{
	try
	{
		if (ConfirmDisbaleActiveScan())
		{
			if (WriteRegistryEntryOfMenuItem(L"dwActiveScanOption", 2))
			{
				m_bActiveScanOption = TRUE;
			}
			if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
			{
				AddLogEntry(L"### Failed to SendData to UI CSystemTray::OnClickDisableActiveScanFor15Min", 0, 0, true, SECONDLEVEL);
			}
			if (!SendData2Service(HANDLEACTICESCANSETTINGS, DISABLEACTSCAN, DISABLE_FOR_15_MINS, 0, 0, true))
			{
				AddLogEntry(L"### Failed to SendData to Service", 0, 0, true, SECONDLEVEL);
			}
			if (!DisplayWidgetsState(REDFLAG))
			{
				AddLogEntry(L"### Failed DisplayWidgetsState REDFLAG in CSystemTray::OnClickDisableActiveScanFor15Min", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::OnClickDisableActiveScanFor15Min", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnClickDisableActiveScanFor1Hr
Description    : Disables active scan feature for .
Author Name    : Nitin Kolapkar
Date           : 13th July 2016
SR_NO		   :
***********************************************************************************************/
void CSystemTray::OnClickDisableActiveScanFor1Hr()
{
	try
	{
		if (ConfirmDisbaleActiveScan())
		{
			if (WriteRegistryEntryOfMenuItem(L"dwActiveScanOption", 3))
			{
				m_bActiveScanOption = TRUE;
			}
			if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
			{
				AddLogEntry(L"### Failed to SendData to UI CSystemTray::OnClickDisableActiveScanFor1Hr", 0, 0, true, SECONDLEVEL);
			}
			if (!SendData2Service(HANDLEACTICESCANSETTINGS, DISABLEACTSCAN, DISABLE_FOR_1_HOUR, 0, 0, true))
			{
				AddLogEntry(L"### Failed to SendData to Service", 0, 0, true, SECONDLEVEL);
			}
			if (!DisplayWidgetsState(REDFLAG))
			{
				AddLogEntry(L"### Failed DisplayWidgetsState REDFLAG in CSystemTray::OnClickDisableActiveScanFor1Hr", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::OnClickDisableActiveScanFor1Hr", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnClickDisableActiveScanUntillReboot
Description    : Disables active scan feature for .
Author Name    : Nitin Kolapkar
Date           : 13th July 2016
SR_NO		   :
***********************************************************************************************/
void CSystemTray::OnClickDisableActiveScanUntillReboot()
{
	try
	{
		if (ConfirmDisbaleActiveScan())
		{
			if (WriteRegistryEntryOfMenuItem(L"dwActiveScanOption", 4))
			{
				m_bActiveScanOption = TRUE;
			}
			if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
			{
				AddLogEntry(L"### Failed to SendData to UI CSystemTray::OnClickDisableActiveScanUntillReboot", 0, 0, true, SECONDLEVEL);
			}
			if (!SendData2Service(HANDLEACTICESCANSETTINGS, DISABLEACTSCAN, DISABLE_UNTIL_RESTART, 0, 0, true))
			{
				AddLogEntry(L"### Failed to SendData to Service", 0, 0, true, SECONDLEVEL);
			}
			if (!DisplayWidgetsState(REDFLAG))
			{
				AddLogEntry(L"### Failed DisplayWidgetsState REDFLAG in CSystemTray::OnClickDisableActiveScanUntillReboot", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::OnClickDisableActiveScanUntillReboot", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : OnClickDisableActiveScanPermanentely
Description    : Disables active scan feature for .
Author Name    : Nitin Kolapkar
Date           : 13th July 2016
SR_NO		   :
***********************************************************************************************/
void CSystemTray::OnClickDisableActiveScanPermanentely()
{
	try
	{
		if (ConfirmDisbaleActiveScan())
		{
			if (WriteRegistryEntryOfMenuItem(L"dwActiveScanOption", 0))
			{
				m_bActiveScanOption = TRUE;
			}
			if (!SendData2UI(SEND_ACTIVE_PROTECTION_STATUS, false))
			{
				AddLogEntry(L"### Failed to SendData to UI CSystemTray::OnClickDisableActiveScanPermanentely", 0, 0, true, SECONDLEVEL);
			}
			if (!SendData2Service(HANDLEACTICESCANSETTINGS, DISABLEACTSCAN, DISABLE_PERMANANT, 0, 0, true))
			{
				AddLogEntry(L"### Failed to SendData to Service", 0, 0, true, SECONDLEVEL);
			}
			if (!DisplayWidgetsState(REDFLAG))
			{
				AddLogEntry(L"### Failed DisplayWidgetsState REDFLAG in CSystemTray::OnClickDisableActiveScanPermanentely", 0, 0, true, SECONDLEVEL);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::OnClickDisableActiveScanPermanentely", 0, 0, true, SECONDLEVEL);
	}
}
bool CSystemTray::ConfirmDisbaleActiveScan()
{
	try
	{
		if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_ACTIVE_SCAN_DISABLE_PROMT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO|MB_ICONWARNING) == IDYES)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CSystemTray::ConfirmDisbaleActiveScan", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : OnClickDisableActiveScanPermanentely
Description    : Disables active scan feature for .
Author Name    : Nitin Kolapkar
Date           : 13th July 2016
SR_NO		   :
***********************************************************************************************/
bool CSystemTray::SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam, LPTSTR lpszSecondParam, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessage;
		szPipeData.dwValue = dwValue;

		if (lpszFirstParam != NULL)
		{
			wcscpy_s(szPipeData.szFirstParam, lpszFirstParam);
		}

		if (lpszSecondParam != NULL)
		{
			wcscpy_s(szPipeData.szSecondParam, lpszSecondParam);
		}

		szPipeData.dwSecondValue = dwSeondValue;

		CISpyCommunicator objCom(SERVICE_SERVER, bWait, 3);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			CString csMessage;
			csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
			AddLogEntry(L"### Failed to set data in CSystemTray::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csMessage;
				csMessage.Format(L"Message: [%d], FirstValue: [%d], SecondValue: [%d]", iMessage, dwValue, dwSeondValue);
				AddLogEntry(L"### Failed to set data in CSystemTray::SendData2Service, %s", csMessage, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in CSystemTray : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : DisplayWidgetsState
Description    : Displaying active scan state for widget.
Author Name    : Amol J.
Date           : 23th Oct 2017
***********************************************************************************************/
bool CSystemTray::DisplayWidgetsState(DWORD dwState)
{
	try
	{
		CISpyAVTrayDlg *pMainWnd = (CISpyAVTrayDlg *)AfxGetMainWnd();
		
		if (!pMainWnd)
			return false;

		if (pMainWnd->m_pWardWizWidgetDlg == NULL)
		{
			return false;
		}
		if (dwState == GREENFLAG)
		{
			pMainWnd->WidgetActiveScanState(GREENFLAG);
			return true;
		}
		else if (dwState == REDFLAG)
		{
			pMainWnd->WidgetActiveScanState(REDFLAG);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to set data in CSystemTray : DisplayWidgetsState", 0, 0, true, SECONDLEVEL);
	}
	return true;
}