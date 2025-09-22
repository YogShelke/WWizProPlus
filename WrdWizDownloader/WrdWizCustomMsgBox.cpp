// WrdWizCustomMsgBox.cpp : Customize the message box for offline patches.
/***************************************************************
*  Program Name: WrdWizCustomMsgBox.cpp
*  Description: Cutomize message box for offline patches.
*  Author Name: Lalit kumawat
*  Date Of Creation: 9-3-2015
*  Version No: 1.10.0.0
*****************************************************************/

#include "stdafx.h"
#include "WrdWizDownloader.h"
#include "WrdWizCustomMsgBox.h"
//#include "afxdialogex.h"



// CWrdWizCustomMsgBox dialog

IMPLEMENT_DYNAMIC(CWrdWizCustomMsgBox, CDialog)


/***************************************************************************************************
*  Function Name  : CWrdWizCustomMsgBox
*  Description    : C'tor
*  Author Name    : Lalit kumawat
*  SR_NO
*  Date           : 
****************************************************************************************************/
CWrdWizCustomMsgBox::CWrdWizCustomMsgBox(CWnd* pParent /*=NULL*/)
: CDialog(CWrdWizCustomMsgBox::IDD, pParent)
, m_stHyperlinkForOfflinePatches(theApp.m_hInstance)
, m_dwMsgErrorNo(0x00)
{

}

/***************************************************************************************************
*  Function Name  : ~CWrdWizCustomMsgBox
*  Description    : D'tor
*  Author Name    : Lalit kumawat
*  SR_NO
*  Date           : 9-3-2015
****************************************************************************************************/
CWrdWizCustomMsgBox::~CWrdWizCustomMsgBox()
{
}

/***********************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Lalit kumawat
*  SR_NO
*  Date           : 9-3-2015
*************************************************************************************************/
void CWrdWizCustomMsgBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_BtnCancel);
	DDX_Control(pDX, IDC_STATIC_ERROR_MSG_ICON, m_stErrorMsgICon);
	DDX_Control(pDX, IDC_STATIC_HYPERLINKFORSITES, m_stHyperlinkForOfflinePatches);
	DDX_Control(pDX, IDC_STATIC_FAILED_MSG, m_stFailedTextMessage);
	//DDX_Text(pDX, IDC_STATIC_FAILED_MSG, m_csFailedMsgText);
}

/***********************************************************************************************
*  Function Name  : MAPPING MESSAGES
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Lalit kumawat
*  SR_NO
*  Date           : 9-3-2015
*************************************************************************************************/
BEGIN_MESSAGE_MAP(CWrdWizCustomMsgBox, CDialog)
	ON_BN_CLICKED(IDOK, &CWrdWizCustomMsgBox::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWrdWizCustomMsgBox::OnBnClickedCancel)
	ON_MESSAGE(_HYPERLINK_EVENT, OnChildFire)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CWrdWizCustomMsgBox message handlers

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global
dialog-box procedure common to all Microsoft Foundation Class Library
dialog boxes
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           :9-3-2015
****************************************************************************************************/
BOOL CWrdWizCustomMsgBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 550, 150, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE);

	CRect rect;
	this->GetClientRect(rect);

	m_bmpErrorIcon = LoadBitmapW(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_MSGICON));
	m_stErrorMsgICon.SetBitmap(m_bmpErrorIcon);
	m_stErrorMsgICon.SetWindowPos(&wndTop, rect.left + 20, 20, 42, 39, SWP_SHOWWINDOW);

	m_bmpMsgBoxImage.LoadBitmapW(IDB_BITMAP_BKGD_CUSTOM_MSG_BOX);

	this->SetWindowTextW(L"WardWiz Download Manager");

	//Neha Gharge 13 May 2015 , Changes in displaying message properly
	if (m_dwMsgErrorNo == 0x01)
	{
		m_stFailedTextMessage.SetWindowPos(&wndTop, rect.left + 78, 10, 550, 60, SWP_NOREDRAW);
	}
	else
	{
		m_stFailedTextMessage.SetWindowPos(&wndTop, rect.left + 78, 25, 550, 30, SWP_NOREDRAW);
	}

	//m_stFailedTextMessage.SetFont(&theApp.m_fontWWTextNormal);
	m_stFailedTextMessage.SetWindowTextW(m_csFailedMsgText);

	if (m_dwMsgErrorNo == 0x01)
	{
		m_stHyperlinkForOfflinePatches.SetWindowPos(&wndTop, rect.left + 78, 55, 450, 20, SWP_NOREDRAW);
	}
	else
	{
		m_stHyperlinkForOfflinePatches.SetWindowPos(&wndTop, rect.left + 78, 45, 450, 20, SWP_NOREDRAW);
	}
	//m_stHyperlinkForOfflinePatches.SetFont(&theApp.m_fontWWTextNormal);
	m_stHyperlinkForOfflinePatches.SetFireChild(1);

	DisplayLinkAsperOSnadProductVersion();

	m_BtnCancel.SetWindowPos(&wndTop, rect.left + 468, 92, 65, 21, SWP_SHOWWINDOW);
	m_BtnCancel.SetWindowTextW(L"Cancel");

	m_btnOK.SetWindowPos(&wndTop, rect.left + 395, 92, 65, 21, SWP_SHOWWINDOW);
	m_btnOK.SetWindowTextW(L"OK");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************
*  Function Name  : OnBnClickedOk
*  Description    : It closes dialog.
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
void CWrdWizCustomMsgBox::OnBnClickedOk()
{
	try
	{
		CDialog::OnOK();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnBnClickedOk", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBnClickedCancel
*  Description    : It closes dialog
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
void CWrdWizCustomMsgBox::OnBnClickedCancel()
{
	try
	{
		CDialog::OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnBnClickedCancel", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnBackground
*  Description    : It will draw the image in background and also stretch if we set bisStretch = true.
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
void CWrdWizCustomMsgBox::OnBackground(CBitmap* bmpImg, CDC* pDC, int yHight, bool bisStretch)
{
	try
	{
		CBitmap bmpObj;
		CRect rect;
		CDC visibleDC;
		//int iOldStretchBltMode;
		GetClientRect(&rect);
		visibleDC.CreateCompatibleDC(pDC);
		bmpObj.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		visibleDC.SelectObject(bmpImg);

		if (bisStretch)
		{
			for (int i = 0; i <= rect.Width() / 2; i++)
			{
				//pDC->StretchBlt(i*5, 0, rect.Width(), rect.Height(), &srcDC, 0, 0, 1000,500, SRCCOPY);
				pDC->BitBlt(i * 2, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
			}
		}
		else
		{
			pDC->BitBlt(0, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}

		bmpObj.DeleteObject();

		visibleDC.DeleteDC();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnBackground", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************************************
*  Function Name  : OnPaint
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 

****************************************************************************************************/
void CWrdWizCustomMsgBox::OnPaint()
{
	try
	{
		CPaintDC dc(this); // device context for painting
		if (IsIconic())
		{
			CPaintDC dc(this); // device context for painting

			SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

			// Center icon in client rectangle
			int cxIcon = GetSystemMetrics(SM_CXICON);
			int cyIcon = GetSystemMetrics(SM_CYICON);
			CRect rect;
			GetClientRect(&rect);
			int x = (rect.Width() - cxIcon + 1) / 2;
			int y = (rect.Height() - cyIcon + 1) / 2;

			// Draw the icon
			//dc.DrawIcon(x, y, m_hIcon);
		}
		else
		{

			OnBackground(&m_bmpMsgBoxImage, &dc, -43, true);

			CDialog::OnPaint();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnPaint", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnChildFire
*  Description    : This WM_MESSAGE which is called after clicking on hyperlink.
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 
****************************************************************************************************/
LRESULT CWrdWizCustomMsgBox::OnChildFire(WPARAM wparam, LPARAM lparam)
{
	try
	{
		TCHAR	szPath[512] = { 0 };

		if (GetDefaultHTTPBrowser(szPath))
		{
			GetEnvironmentVariable(L"ProgramFiles", szPath, 511);
			wcscat_s(szPath, 511, L"\\Internet Explorer\\iexplore.exe");
		}

		bool bGetOSType = false;//GetOSType();
		
		ShellExecute(NULL, L"Open", szPath, theApp.m_csurl, NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnChildFire", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : OnCtlColor
*  Description    : Give traspaent background to give controls
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
HBRUSH CWrdWizCustomMsgBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	try
	{


		int ctrlID;
		ctrlID = pWnd->GetDlgCtrlID();
		if (ctrlID == IDC_STATIC_FAILED_MSG
			)

		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::OnCtlColor", 0, 0, true, SECONDLEVEL);
	}
	return hbr;
}

/***************************************************************************************************
*  Function Name  : GetOSType
*  Description    : It will return true if client machine is 64 bit and false if 32 bit
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
bool CWrdWizCustomMsgBox::GetOSType()
{
	try
	{
	//	WardWizSystemInfo objSystemInfo;
	//	return objSystemInfo.GetOSType();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::GetOSType", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DisplayLinkAsperOSnadProductVersion
*  Description    : Display the link as per client OS version and Product ID.
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
void CWrdWizCustomMsgBox::DisplayLinkAsperOSnadProductVersion()
{
	try
	{
		m_stHyperlinkForOfflinePatches.SetWindowTextW(theApp.m_csurl);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::DisplayLinkAsperOSnadProductVersion", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : PreTranslateMessage
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 9-3-2015
****************************************************************************************************/
BOOL CWrdWizCustomMsgBox::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE))
		{
			return TRUE;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizCustomMsgBox::PreTranslateMessage", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::PreTranslateMessage(pMsg);
}



/***************************************************************************
Function Name  : GetDefaultHTTPBrowser
Description    : Function which gets default browser present on client machine
Author Name    :
SR_NO		   :
Date           : 9-3-2015
****************************************************************************/
bool CWrdWizCustomMsgBox::GetDefaultHTTPBrowser(LPTSTR lpszBrowserPath)
{
	try
	{

		CITinRegWrapper	objReg;

		TCHAR	szPath[512] = { 0 };
		DWORD	dwSize = 511;

		objReg.GetRegistryValueData(HKEY_CURRENT_USER, L"Software\\Classes\\http\\shell\\open\\command", L"", szPath, dwSize);
		if (!szPath[0])
			return true;

		_wcsupr(szPath);

		TCHAR	*pTemp = StrStr(szPath, L".EXE");

		if (!pTemp)
			return true;

		pTemp += wcslen(L".EXE") + 0x01;

		*pTemp = '\0';

		if (szPath[wcslen(szPath) - 1] == '"')
			szPath[wcslen(szPath) - 1] = '\0';

		wcscpy_s(lpszBrowserPath, 511, &szPath[1]);

		if (!PathFileExists(lpszBrowserPath))
			return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to Domodal Support Window", 0, 0, true, SECONDLEVEL);
	}
	return false;
}
