/////////////////////////////////////////////////////////////////////////////
// SystemTray.h : header file
//
// Written by Chris Maunder (cmaunder@mail.com)
// Copyright (c) 1998.
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
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 

#ifndef _INCLUDED_SYSTEMTRAY_H_
#define _INCLUDED_SYSTEMTRAY_H_

#ifdef NOTIFYICONDATA_V1_SIZE   // If NOTIFYICONDATA_V1_SIZE, then we can use fun stuff
#define SYSTEMTRAY_USEW2K
#else
#define NIIF_NONE 0
#endif

// #include <afxwin.h>
#include <afxtempl.h>
#include <afxdisp.h>    // COleDateTime
#include "MenuXP.h"
#include "resource.h"
#include "ISpyCommunicator.h"
#include "PipeConstants.h"


/////////////////////////////////////////////////////////////////////////////
// CSystemTray window

class CSystemTray : public CWnd
{
// Construction/destruction
public:
    CSystemTray();
    CSystemTray(CWnd* pWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID, 
                BOOL bhidden = FALSE,
                LPCTSTR szBalloonTip = NULL, LPCTSTR szBalloonTitle = NULL, 
                DWORD dwBalloonIcon = NIIF_NONE, UINT uBalloonTimeout = 10);
    virtual ~CSystemTray();

    DECLARE_DYNAMIC(CSystemTray)

// Operations
public:
    BOOL Enabled() { return m_bEnabled; }
    BOOL Visible() { return !m_bHidden; }

    // Create the tray icon
    BOOL Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID,
                BOOL bHidden = FALSE,
                LPCTSTR szBalloonTip = NULL, LPCTSTR szBalloonTitle = NULL, 
                DWORD dwBalloonIcon = NIIF_NONE, UINT uBalloonTimeout = 10);

    // Change or retrieve the Tooltip text
    BOOL    SetTooltipText(LPCTSTR pszTooltipText);
    BOOL    SetTooltipText(UINT nID);
    CString GetTooltipText() const;

    // Change or retrieve the icon displayed
    BOOL  SetIcon(HICON hIcon);
    BOOL  SetIcon(LPCTSTR lpszIconName);
    BOOL  SetIcon(UINT nIDResource);
    BOOL  SetStandardIcon(LPCTSTR lpIconName);
    BOOL  SetStandardIcon(UINT nIDResource);
    HICON GetIcon() const;

    void  SetFocus();
    BOOL  HideIcon();
    BOOL  ShowIcon();
    BOOL  AddIcon();
    BOOL  RemoveIcon();
    BOOL  MoveToRight();

    BOOL ShowBalloon(LPCTSTR szText, LPCTSTR szTitle = NULL,
                     DWORD dwIcon = NIIF_NONE, UINT uTimeout = 10);

    // For icon animation
    BOOL  SetIconList(UINT uFirstIconID, UINT uLastIconID); 
    BOOL  SetIconList(HICON* pHIconList, UINT nNumIcons); 
    BOOL  Animate(UINT nDelayMilliSeconds, int nNumSeconds = -1);
    BOOL  StepAnimation();
    BOOL  StopAnimation();

    // Change menu default item
    void GetMenuDefaultItem(UINT& uItem, BOOL& bByPos);
    BOOL SetMenuDefaultItem(UINT uItem, BOOL bByPos);


	 BOOL CheckMenuItem(UINT uiCmd, BOOL bCheck);
 

    // Change or retrieve the window to send notification messages to
    BOOL  SetNotificationWnd(CWnd* pNotifyWnd);
    CWnd* GetNotificationWnd() const;

    // Change or retrieve the window to send menu commands to
    BOOL  SetTargetWnd(CWnd* pTargetWnd);
    CWnd* GetTargetWnd() const;

    // Change or retrieve  notification messages sent to the window
    BOOL  SetCallbackMessage(UINT uCallbackMessage);
    UINT  GetCallbackMessage() const;

    UINT_PTR GetTimerID() const   { return m_nTimerID; }



		
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);



// Static functions
public:
    static void MinimiseToTray(CWnd* pWnd, BOOL bForceAnimation = FALSE);
    static void MaximiseFromTray(CWnd* pWnd, BOOL bForceAnimation = FALSE);

public:
    // Default handler for tray notification message
    virtual LRESULT OnTrayNotification(UINT wParam, LONG lParam);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSystemTray)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
    void Initialise();
    void InstallIconPending();

	virtual void CustomizeMenu(CMenu*) {}	// Used for customizing the menu

// Implementation
protected:
    NOTIFYICONDATA  m_tnd;
    BOOL            m_bEnabled;         // does O/S support tray icon?
    BOOL            m_bHidden;          // Has the icon been hidden?
    BOOL            m_bRemoved;         // Has the icon been removed?
    BOOL            m_bShowIconPending; // Show the icon once tha taskbar has been created
    BOOL            m_bWin2K;           // Use new W2K features?
	CWnd*           m_pTargetWnd;       // Window that menu commands are sent

    CArray<HICON, HICON> m_IconList;
    UINT_PTR     m_uIDTimer;
    INT_PTR      m_nCurrentIcon;
    COleDateTime m_StartTime;
    UINT         m_nAnimationPeriod;
    HICON        m_hSavedIcon;
    UINT         m_DefaultMenuItemID;
    BOOL         m_DefaultMenuItemByPos;
	UINT         m_uCreationFlags;
	CMenu        m_Menu;
	CString		 m_Key;
	// Static data
protected:
    static BOOL RemoveTaskbarIcon(CWnd* pWnd);

    static const UINT m_nTimerID;
    static UINT  m_nMaxTooltipLength;
    static const UINT m_nTaskbarCreatedMsg;
    static CWnd  m_wndInvisible;

    static BOOL GetW2K();
#ifndef _WIN32_WCE
    static void GetTrayWndRect(LPRECT lprect);
    static BOOL GetDoWndAnimation();
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CSystemTray)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
#ifndef _WIN32_WCE
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
#endif
    LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);

//Initializing the CONSTANTS FOR MENU ITEMS
#define ID_ITIN_AV 							1
#define ID_EMAIL_SCAN 						2
#define ID_ANTI_ROOTKIT 					3
#define ID_USB_DETECT 						4
#define ID_EXIT 							5
#define ID_AUTOUPDATE						6
#define	ID_TRAY_NOTIFICATION				7
#define	ID_STARTUP_SCAN						8
#define	ID_ENABLE_ACTIVE_SCAN				9
#define ID_DIS_ACTIVE_SCAN_15_MINS			10
#define ID_DIS_ACTIVE_SCAN_1_HR				11
#define ID_DIS_ACTIVE_SCAN_UNTILL_REBOOT	12
#define ID_DIS_ACTIVE_SCAN_PERMANENTLY		13

public:

	/* Nitin K 25 June 2014 AV Tray Functionalities */
	CMenuXP	*m_sMenu;	
	//Boolean Flag for CHECK/UNCHECK
	BOOL m_bEmailScan;
	BOOL m_bAntiRootkit;
	BOOL m_bUsbDetect;
	BOOL m_bAutoUpdate;
	BOOL m_bTrayNotification;
	BOOL m_bStartUpScan;
	BOOL m_bActiveScanOption;
	

	BOOL m_bLazyWrite;
	CString m_strCurrentPath;


	bool LaunchISpyUI();
	void ContextMenu(CPoint pos);//Launching Menu on Mouse Right Click
	
	//Declaration for Tray Icon Menu Methods
	afx_msg void OnPopupItinantivirus();//Launching application from Menu ITEM
	afx_msg void OnPopupEmailScan();//To set CHECK/UNCHECK flag for MENU ITEM
	afx_msg void OnPopupAntirootkit();//To set CHECK/UNCHECK flag for MENU ITEM
	afx_msg void OnPopupUsbdetect();//To set CHECK/UNCHECK flag for MENU ITEM
	afx_msg void OnPopupAutoUpdate();//To set CHECK/UNCHECK flag for MENU ITEM
	afx_msg void OnPopupTrayNotification();//To set CHECK/UNCHECK flag for MENU ITEM
	afx_msg void OnPopupStartupScan();//To set CHECK/UNCHECK flag for MENU ITEM
	afx_msg void OnPopupExit();//Exits the application
	afx_msg void OnClickEnableActiveScan();
	afx_msg void OnClickDisableActiveScanFor15Min();
	afx_msg void OnClickDisableActiveScanFor1Hr();
	afx_msg void OnClickDisableActiveScanUntillReboot();
	afx_msg void OnClickDisableActiveScanPermanentely();
	
	DWORD ReadRegistryEntryofMenuItem(CString strKey);//Read DWORD value from Registry
	BOOL WriteDword(CString strKey,DWORD value);//Write DWORD value to Registry
	void SetRegistryFlag();// Set the Menu Item Flags to Either CHECKED/UNCHECK depending on Registry values
	BOOL WriteRegistryEntryOfMenuItem(CString strKey,DWORD dwChangeValue);//Write the Registry Key Value for Menu Items
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait = false);//Set the Registry Key Value for Menu Items using Service

	void OnPopupEmailScanSubOptions(DWORD FlagValue);
	BOOL WriteRegistryEntryOfEmailScanSubOptions(LPCTSTR SubKey,CString strKey, DWORD dwChangeValue);
	bool SendEmailPluginChange2EmailPlugin(int iMessageInfo, DWORD dwType);
	HRESULT GetOutlookVersionString(LPTSTR* ppszVer, BOOL* pf64Bit);
	bool GetExistingPathofOutlook();
	bool CheckForOutllookExe();
	bool SendData2UI(int iMessageInfo, bool bWait = false);
	bool SendRequestALUservToStopUpdate(int iRequest);
	bool SendData2Service(int iMessage, DWORD dwValue, DWORD dwSeondValue, LPTSTR lpszFirstParam = NULL, LPTSTR lpszSecondParam = NULL, bool bWait = false);
	bool ConfirmDisbaleActiveScan();
	bool DisplayWidgetsState(DWORD dwState);

	DWORD				m_dwProductID;
	bool				m_bOutlookFlag;
	bool				m_bExitPopupflag;
	
	DECLARE_MESSAGE_MAP()
};

typedef enum
{
	GREENFLAG,
	YELLOWLAG,
	REDFLAG,
}WIDGET_STATE;

#endif

/////////////////////////////////////////////////////////////////////////////
