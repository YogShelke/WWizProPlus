// WardWizUtilitiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizUtilitiesDlg.h"
#include "afxdialogex.h"
#include "ISpyGUI.h"


// CWardWizUtilitiesDlg dialog

IMPLEMENT_DYNAMIC(CWardWizUtilitiesDlg, CDialogEx)

/***************************************************************************
Function Name  : CWardWizUtilitiesDlg
Description    : Constructor
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
CWardWizUtilitiesDlg::CWardWizUtilitiesDlg(CWnd* pParent /*=NULL*/)
: CJpegDialog(CWardWizUtilitiesDlg::IDD, pParent)
{

}

/***************************************************************************
Function Name  : CWardWizUtilitiesDlg
Description    : Constructor
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
CWardWizUtilitiesDlg::~CWardWizUtilitiesDlg()
{
}

/***************************************************************************
Function Name  : CWardWizUtilitiesDlg
Description    : Constructor
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
void CWardWizUtilitiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HEADER_UTILITY, m_stHeaderDescription);
	DDX_Control(pDX, IDC_STATIC_HEADER_PIC_UTILITY, m_stHUtility);
	DDX_Control(pDX, IDC_STATIC_HEADER_NAME_UTILITY, m_stHeaderName);
	DDX_Control(pDX, IDC_STATIC_AUTORUN_HEADER, m_stAutorunHeader);
	DDX_Control(pDX, IDC_STATIC_AUTORUN_DESC, m_stAutoRunDesc);
	DDX_Control(pDX, IDC_STATIC_TEMPFILE_HEADER, m_stTempFileHeader);
	DDX_Control(pDX, IDC_STATIC_TEMPFILE_DESC, m_stTempFileDesc);
	DDX_Control(pDX, IDC_STATIC_USBVAC_HEADER, m_stUSBVacHeader);
	DDX_Control(pDX, IDC_STATIC_USBVAC_DESC, m_stUSBVacDesc);
	DDX_Control(pDX, IDC_BUTTON_AUTORUN, m_btnAutoRunScan);
	DDX_Control(pDX, IDC_BUTTON_TEMPFILE_CLEANER, m_btnTempCleaner);
	DDX_Control(pDX, IDC_BUTTON_USBVAC, m_btnUSBVac);
}

/***************************************************************************
Function Name  : CWardWizUtilitiesDlg
Description    : Constructor
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
BEGIN_MESSAGE_MAP(CWardWizUtilitiesDlg, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_AUTORUN, &CWardWizUtilitiesDlg::OnBnClickedButtonAutorun)
	ON_BN_CLICKED(IDC_BUTTON_TEMPFILE_CLEANER, &CWardWizUtilitiesDlg::OnBnClickedButtonTempfileCleaner)
	ON_BN_CLICKED(IDC_BUTTON_USBVAC, &CWardWizUtilitiesDlg::OnBnClickedButtonUsbvac)
END_MESSAGE_MAP()


// CWardWizUtilitiesDlg message handlers

/***************************************************************************
Function Name  : CWardWizUtilitiesDlg
Description    : Constructor
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
BOOL CWardWizUtilitiesDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if (!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_DATA_ENCRYPTION_BG), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}

	Draw();

	CRect	rect;
	this->GetClientRect(rect);

	CRgn RgnRect;
	RgnRect.CreateRectRgn(rect.left, rect.top, rect.right , rect.bottom);
	this->SetWindowRgn(RgnRect, TRUE);

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL, MAKEINTRESOURCE(IDC_CURSOR_HAND));

	// Issue Add description Neha Gharge 9-3-2015
	m_stHeaderDescription.SetWindowPos(&wndTop, rect.left + 20, 35, 400, 15, SWP_NOREDRAW);
	m_stHeaderDescription.SetTextColor(RGB(24, 24, 24));
	m_stHeaderDescription.SetBkColor(RGB(230, 232, 238));
	m_stHeaderDescription.SetFont(&theApp.m_fontWWTextSubTitleDescription);

	m_stHeaderName.SetWindowPos(&wndTop, rect.left + 20, 11, 540, 31, SWP_NOREDRAW);
	m_stHeaderName.SetTextColor(RGB(24, 24, 24));
	m_stHeaderName.SetBkColor(RGB(230, 232, 238));
	m_stHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);

	m_bmpHUtility = LoadBitmapW(theApp.m_hResDLL, MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHUtility.SetWindowPos(&wndTop, rect.left + 6, 10, 586, 45, SWP_NOREDRAW);
	m_stHUtility.SetBitmap(m_bmpHUtility);

	m_btnAutoRunScan.SetSkin(theApp.m_hResDLL, IDB_BITMAP_AUTORUN_SCANNER, IDB_BITMAP_AUTORUN_SCANNER, IDB_BITMAP_H_AUTORUN_SCANNER, IDB_BITMAP_AUTORUN_SCANNER, 0, 0, 0, 0, 0);
	m_btnAutoRunScan.SetWindowPos(&wndTop, rect.left + 30, 100, 79, 60, SWP_NOREDRAW);

	m_btnTempCleaner.SetSkin(theApp.m_hResDLL, IDB_BITMAP_TEMP_FILE_CLEANER, IDB_BITMAP_TEMP_FILE_CLEANER, IDB_BITMAP_H_TEMP_FILE_CLEANER, IDB_BITMAP_TEMP_FILE_CLEANER, 0, 0, 0, 0, 0);
	m_btnTempCleaner.SetWindowPos(&wndTop, rect.left + 30, 190, 79, 60, SWP_NOREDRAW);

	m_btnUSBVac.SetSkin(theApp.m_hResDLL, IDB_BITMAP_USB_VAC, IDB_BITMAP_USB_VAC, IDB_BITMAP_H_USB_VAC, IDB_BITMAP_USB_VAC, 0, 0, 0, 0, 0);
	m_btnUSBVac.SetWindowPos(&wndTop, rect.left + 30, 285, 79, 60, SWP_NOREDRAW);

	m_stAutorunHeader.SetTextColor(RGB(24, 24, 24));
	m_stAutorunHeader.SetBkColor(RGB(255, 255, 255));
	m_stAutorunHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stAutorunHeader.SetWindowPos(&wndTop, rect.left + 170, 100, 300, 25, SWP_NOREDRAW);
	m_stAutorunHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_AUTORUNSCAN"));

	m_stAutoRunDesc.SetTextColor(RGB(24, 24, 24));
	m_stAutoRunDesc.SetBkColor(RGB(255, 255, 255));
	m_stAutoRunDesc.SetWindowPos(&wndTop, rect.left + 170, 130, 400, 40, SWP_NOREDRAW);
	m_stAutoRunDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_AUTORUNSCAN_DESC"));

	m_stTempFileHeader.SetTextColor(RGB(24, 24, 24));
	m_stTempFileHeader.SetBkColor(RGB(255, 255, 255));
	m_stTempFileHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stTempFileHeader.SetWindowPos(&wndTop, rect.left + 170, 190, 300, 25, SWP_NOREDRAW);
	m_stTempFileHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_TEMPCLEANER"));

	m_stTempFileDesc.SetTextColor(RGB(24, 24, 24));
	m_stTempFileDesc.SetBkColor(RGB(255, 255, 255));
	m_stTempFileDesc.SetWindowPos(&wndTop, rect.left + 170, 220, 400, 40, SWP_NOREDRAW);
	m_stTempFileDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_TEMPCLEANER_DESC"));

	m_stUSBVacHeader.SetTextColor(RGB(24, 24, 24));
	m_stUSBVacHeader.SetBkColor(RGB(255, 255, 255));
	m_stUSBVacHeader.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stUSBVacHeader.SetWindowPos(&wndTop, rect.left + 170, 290, 300, 25, SWP_NOREDRAW);
	m_stUSBVacHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_USBVACCINATOR"));

	m_stUSBVacDesc.SetTextColor(RGB(24, 24, 24));
	m_stUSBVacDesc.SetBkColor(RGB(255, 255, 255));
	m_stUSBVacDesc.SetWindowPos(&wndTop, rect.left + 170, 320, 400, 40, SWP_NOREDRAW);
	m_stUSBVacDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_USBVACCINATOR_DESC"));

	RefreshStrings();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************
Function Name  : CWardWizUtilitiesDlg
Description    : Constructor
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
LRESULT CWardWizUtilitiesDlg::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return HTCLIENT;
}

/**********************************************************************************************************
*  Function Name  :	RefreshStrings
*  Description    : Refresh all strings
*  SR.N0		  :
*  Author Name    :	Neha Gharge
*  Date           : 27 May 2015
**********************************************************************************************************/
void CWardWizUtilitiesDlg::RefreshStrings()
{
	// Issue Add description Neha Gharge 9-3-2015
	m_stHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_HEADER"));
	m_stHeaderDescription.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_HEADER_DESC"));
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : To translate window messages before they are dispatched 
				 to the TranslateMessage and DispatchMessage Windows functions
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
BOOL CWardWizUtilitiesDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnSetCursor
Description    : The framework calls this member function if mouse input 
				 is not captured and the mouse causes cursor movement within the CWnd object.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
BOOL CWardWizUtilitiesDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!pWnd)
		return FALSE;

	// hand cursor on information button neha gharge 22/5/2014.
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if (
		iCtrlID == IDC_BUTTON_AUTORUN ||
		iCtrlID == IDC_BUTTON_TEMPFILE_CLEANER ||
		iCtrlID == IDC_BUTTON_USBVAC 
		)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if (csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}

	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/***************************************************************************
Function Name  : OnBnClickedButtonAutorun
Description    : Launch the auto run scan.
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
void CWardWizUtilitiesDlg::OnBnClickedButtonAutorun()
{
	m_csAppPath = L"";
	m_csAppPath = GetAppFolderPath();
	m_csAppPath += L"\\WRDWIZAUTORUNSCN.EXE";
	ShellExecute(NULL,L"open", m_csAppPath, NULL, NULL, SW_SHOW);
}

/***************************************************************************
Function Name  : OnBnClickedButtonTempfileCleaner
Description    : Launch the Temp file cleaner
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
void CWardWizUtilitiesDlg::OnBnClickedButtonTempfileCleaner()
{
	m_csAppPath = L"";
	m_csAppPath = GetAppFolderPath();
	m_csAppPath += L"\\WRDWIZTEMPCLR.EXE";
	ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
}

/***************************************************************************
Function Name  : OnBnClickedButtonUsbvac
Description    : Launch the USB Vaccinator
Author Name    : Neha Gharge
SR_NO		   :
Date           : 28th May 2015
****************************************************************************/
void CWardWizUtilitiesDlg::OnBnClickedButtonUsbvac()
{
	m_csAppPath = L"";
	m_csAppPath = GetAppFolderPath();
	m_csAppPath += L"\\WRDWIZUSBVAC.EXE";
	ShellExecute(NULL, L"open", m_csAppPath, NULL, NULL, SW_SHOW);
}


/***************************************************************************************************
*  Function Name  :   GetAppFolderPath
*  Description    :   Get App folder path.
*  Author Name    :   Neha Gharge
*  SR_NO		  :
*  Date           :   26 may,2015
****************************************************************************************************/
CString CWardWizUtilitiesDlg::GetAppFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csAppFolderPath = szModulePath;
		return csAppFolderPath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizUtilitiesDlg::GetAppFolderPath()", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}
